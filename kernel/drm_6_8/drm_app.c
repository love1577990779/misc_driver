#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
int main(void)
{
  int fd;
  drmModeConnector *conn;
  drmModeRes *res;
  uint32_t conn_id;
  uint32_t crtc_id;

  fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

  return 0;
}

