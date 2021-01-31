#include "ipkvm.h"

// VNC SERVER

static void vnc_send_framebuffer_update(ipkvm_t *ipkvm, rfbClientPtr cl) {
    if (ipkvm->last_buffer_idx < 0)
        return;

    char *data = ipkvm->buffers[ipkvm->last_buffer_idx].data;
    uint32_t data_size = ipkvm->buffers[ipkvm->last_buffer_idx].payload;

    if (!data)
        return;

    rfbFramebufferUpdateMsg *fu = (rfbFramebufferUpdateMsg *) cl->updateBuf;

    if (cl->enableLastRectEncoding) {
        fu->nRects = 0xFFFF;
    } else {
        fu->nRects = Swap16IfLE(1);
    }

    fu->type = rfbFramebufferUpdate;
    cl->ublen = sz_rfbFramebufferUpdateMsg;
    rfbSendUpdateBuf(cl);

    cl->tightEncoding = rfbEncodingTight;
    rfbSendTightHeader(cl, 0, 0, ipkvm->width, ipkvm->height);

    cl->updateBuf[cl->ublen++] = (char) (rfbTightJpeg << 4);
    rfbSendCompressedDataTight(cl, data, data_size);

    if (cl->enableLastRectEncoding) {
        rfbSendLastRectMarker(cl);
    }

    rfbSendUpdateBuf(cl);
}

rfbBool vnc_update_client(ipkvm_t *ipkvm, rfbClientPtr cl)
{
  struct timeval tv;
  rfbBool result=FALSE;
  rfbScreenInfoPtr screen = cl->screen;

  if (cl->sock != RFB_INVALID_SOCKET && !cl->onHold && FB_UPDATE_PENDING(cl) && !sraRgnEmpty(cl->requestedRegion)) {
      result=TRUE;

#ifdef AUDIO_EXTENSION
    if(ipkvm->audio_enabled) {
        rfb_audio_extension_send(cl, ipkvm->audio_buffer, ipkvm->audio_frames * 16 / 8 * AUDIO_CHANNELS);
    }
#endif

      if(ipkvm->new_frame) {
        vnc_send_framebuffer_update(ipkvm, cl);
        ipkvm->new_frame = false;
       }
    }

    if (!cl->viewOnly && cl->lastPtrX >= 0) {
      if(cl->startPtrDeferring.tv_usec == 0) {
        gettimeofday(&cl->startPtrDeferring,NULL);
        if(cl->startPtrDeferring.tv_usec == 0)
          cl->startPtrDeferring.tv_usec++;
      } else {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        if(tv.tv_sec < cl->startPtrDeferring.tv_sec /* at midnight */
           || ((tv.tv_sec-cl->startPtrDeferring.tv_sec)*1000
           +(tv.tv_usec-cl->startPtrDeferring.tv_usec)/1000)
           > cl->screen->deferPtrUpdateTime) {
          cl->startPtrDeferring.tv_usec = 0;
          cl->screen->ptrAddEvent(cl->lastPtrButtons,
                                  cl->lastPtrX,
                                  cl->lastPtrY, cl);
          cl->lastPtrX = -1;
        }
      }
    }

    return result;
}

rfbBool vnc_process_events(ipkvm_t *ipkvm, long usec)
{
  rfbScreenInfoPtr screen = ipkvm->server;
  rfbClientIteratorPtr i;
  rfbClientPtr cl,clPrev;
  rfbBool result=FALSE;
  // rfbGetClientIteratorWithClosed function is not exposed in API
  extern rfbClientIteratorPtr
    rfbGetClientIteratorWithClosed(rfbScreenInfoPtr rfbScreen);

  rfbCheckFds(screen,usec);
  rfbHttpCheckFds(screen);

  i = rfbGetClientIteratorWithClosed(screen);
  cl=rfbClientIteratorHead(i);
  while(cl) {
    result = vnc_update_client(ipkvm, cl);
    clPrev=cl;
    cl=rfbClientIteratorNext(i);
    if(clPrev->sock==RFB_INVALID_SOCKET) {
      rfbClientConnectionGone(clPrev);
      result=TRUE;
    }
  }
  rfbReleaseClientIterator(i);

  return result;
}

// libvncserver callbacks

static void vnc_client_gone(rfbClientPtr cl) {
    ipkvm_t *ipkvm = cl->screen->screenData;
    ipkvm->need_capture = false;
    ipkvm->audio_enabled = false;
}

static enum rfbNewClientAction vnc_client_new(rfbClientPtr cl) {
    ipkvm_t *ipkvm = cl->screen->screenData;
    cl->clientGoneHook = vnc_client_gone;

    if (ipkvm->need_capture) { // only one client at a time for now
        return RFB_CLIENT_REFUSE;
    } else {
        ipkvm->need_capture = true;
        return RFB_CLIENT_ACCEPT;
    }
}

static void vnc_pointer_event(int buttonMask, int x, int y, rfbClientPtr cl) {
    ipkvm_t *ipkvm = cl->screen->screenData;
    input_pointer_event(ipkvm, buttonMask, x, y);
    rfbDefaultPtrAddEvent(buttonMask, x, y, cl);
}

static void vnc_key_event(rfbBool down, rfbKeySym keysym, rfbClientPtr cl) {
    ipkvm_t *ipkvm = cl->screen->screenData;
    input_keyboard_event(ipkvm, down, keysym);
}

void server_resize(ipkvm_t *ipkvm) {
    ipkvm->v_fb = realloc(ipkvm->v_fb, ipkvm->width * ipkvm->width * 4);
    rfbNewFramebuffer(ipkvm->server, ipkvm->v_fb, ipkvm->width, ipkvm->width, 8, 3, 4);
    rfbMarkRectAsModified(ipkvm->server, 0, 0, ipkvm->width, ipkvm->height);
}

static int vnc_set_desktop_size(int width, int height, int numScreens, struct rfbExtDesktopScreen *extDesktopScreens,
                                struct _rfbClientRec *cl) {
    ipkvm_t *ipkvm = cl->screen->screenData;
    return rfbExtDesktopSize_ResizeProhibited;
}

void server_open(ipkvm_t *ipkvm, char *address) {
#ifdef AUDIO_EXTENSION
    rfb_audio_extension_register();
#endif

    int argc = 0;
    ipkvm->server = rfbGetScreen(&argc, NULL, ipkvm->width, ipkvm->height, 8, 3, 4);

    ipkvm->server->screenData = ipkvm;

    ipkvm->v_fb = malloc(ipkvm->width * ipkvm->height * 4); // fake framebuffer, won't be actually used
    ipkvm->server->frameBuffer = ipkvm->v_fb;

    ipkvm->server->desktopName = "BBB IKVM";
    ipkvm->server->newClientHook = vnc_client_new;
    ipkvm->server->setDesktopSizeHook = vnc_set_desktop_size;

    rfbStringToAddr(address, &ipkvm->server->listenInterface);

    rfbInitServer(ipkvm->server);

    rfbMarkRectAsModified(ipkvm->server, 0, 0, ipkvm->width, ipkvm->height);

    ipkvm->server->ptrAddEvent = vnc_pointer_event;
    ipkvm->server->kbdAddEvent = vnc_key_event;
}

void server_process(ipkvm_t *ipkvm) {
    if (rfbIsActive(ipkvm->server)) {
        vnc_process_events(ipkvm, SERVER_DEFER_US);
    }
}

void server_close(ipkvm_t *ipkvm) {
    free(ipkvm->v_fb);
    rfbScreenCleanup(ipkvm->server);
}
