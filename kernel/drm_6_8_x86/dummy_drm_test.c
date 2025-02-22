#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>

drmModeRes *res = NULL;
drmModePlaneRes *plane_res = NULL;
drmModeCrtc* crtc_status = NULL;
drmModePlane* plane_status = NULL;
drmModeEncoder* encoder_status = NULL;
drmModeConnector* connector_status = NULL;



int crtc_id = 0;
int plane_id = 0;
int encoder_id = 0;
int connector_id = 0;


//struct drm_device drm_dev;

int handle = 0;
int pitch = 0;
int size = 0;
int offset = 0;
int fb_id = 0;
void* vaddr = NULL;



int main(void)
{
  int fd;
	int ret = 0;

  fd = open("/dev/dri/card1", O_RDWR);
	if(0 < fd)
	{
		printf("fd = %d\r\n" , fd);
		res = drmModeGetResources(fd);
		crtc_id = res->crtcs[0];
		crtc_status = drmModeGetCrtc(fd , crtc_id);

		drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
		plane_res = drmModeGetPlaneResources(fd);
		plane_id = plane_res->planes[0];
		plane_status = drmModeGetPlane(fd , plane_id);


		encoder_id = res->encoders[0];
		encoder_status = drmModeGetEncoder(fd , encoder_id);

		connector_id = res->connectors[0];
		connector_status = drmModeGetConnector(fd , connector_id);

		//drm_dev.width = connector_status->modes[0].hdisplay;
		//drm_dev.height = connector_status->modes[0].vdisplay;

		//drm_dev.create.width = drm_dev.width;
		//drm_dev.create.height = drm_dev.height;
		//drm_dev.create.bpp = 32;

		struct drm_mode_create_dumb create = 
		{
			.width = connector_status->modes[0].hdisplay ,
			.height = connector_status->modes[0].vdisplay,
			.bpp = 32,
		};
		
		ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
		
		
		drmModeAddFB(fd, create.width, create.height, 24, 32, create.pitch, create.handle, &fb_id);

		struct drm_mode_map_dumb map = 
		{
			.handle = create.handle,
		};

		drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);

		vaddr = mmap(0, create.size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, map.offset);

		// int bp = 0;
		// bp++;
		drmModeSetCrtc(fd, crtc_id, fb_id, 0, 0, &connector_id, 1, &connector_status->modes[0]);
	}
	else
	{
		printf("fd = %d\r\n" , fd);
	}

  return 0;
}




