#include <unistd.h>  
#include <stdio.h>  
#include <fcntl.h>  
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <stdlib.h>  
#include <string.h>
#include <errno.h>

#define RED    0xF800
#define YELLOW    0xFFE0
#define BLUE     0x001F
#define WHITE    0xFFFF 
#define BLACK    0x0000


struct fb_var_screeninfo vinfo;  
struct fb_fix_screeninfo finfo;

void fill_color16(short *fb_addr, short bit_map, int psize)
{
    int i;
    for(i=0; i<psize; i++) {
        *fb_addr = bit_map;
        fb_addr++;
    }
}


void test_draw_line(unsigned char *fb_addr , int pos_x , int pos_y , int len , int color)
{ 
    int x = pos_x;
    int y = pos_y;
    int location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;  
    unsigned int* test_fbp = fb_addr + location;
    printf("draw line.......\n");
    for(int i = 0; i < len; i++)
    {
    	printf("test_fbp = 0x%x\r\n" , test_fbp);
        *test_fbp++ = color;
	}
	
	
    for(int i = 0 ; i < len ; i++)
    {
    	location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;  
    	test_fbp = fb_addr + location;
    	*test_fbp = color;
    	y++;
    }
    
    sleep(5);
    
    #if 0
    location = pos_x * (vinfo.bits_per_pixel / 8) + pos_y  *  finfo.line_length;  
    test_fbp = fb_addr + location;
    for(int i = 0; i < len; i++)
    {
    	printf("h color = 0x%x" , *test_fbp);
    	test_fbp++;
    }
    
    x = pos_x;
    y = pos_y;
    for(int i = 0; i < len; i++)
    {
    	location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;  
    	test_fbp = fb_addr + location;
    	printf("v color = 0x%x" , *test_fbp);
    	y++;
    	
    }
    #endif
    
    for(unsigned int i = pos_y ; i < pos_y + len ;i++)
    {
    	for(unsigned int k = 0 ; k < vinfo.xres ; k++)
    		printf("(%d , %d) 0x%x = 0x%x\r\n" , k , i , fb_addr + i *finfo.line_length + k *4 , *((unsigned int*)(fb_addr + i *finfo.line_length + k *4)));
    }
    
    
}


int main ()   
{  
    int fp=0;    
    long screensize=0;  
    char *fbp = NULL;
    unsigned int *test_fbp=NULL;    
    int x = 0, y = 0;  
    long location = 0;
    int i;
    int num = 5;
    int pix_size=0;

    fp = open("/dev/fb1", O_RDWR);  

    if(fp < 0) {  
        printf("Error : Can not open framebuffer device/n");  
        exit(1);  
    }  

    if(ioctl(fp, FBIOGET_FSCREENINFO, &finfo)){  
        printf("Error reading fixed information/n");  
        exit(2);  
    }  

    if(ioctl(fp, FBIOGET_VSCREENINFO, &vinfo)){  
        printf("Error reading variable information/n");  
        exit(3);  
    }  

    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;  

    printf("The phy mem = 0x%x, total size = %d(byte)\n", finfo.smem_start, finfo.smem_len); 
    printf("finfo.line_length = %d\n", finfo.line_length); 
    printf("xres =  %d, yres =  %d, bits_per_pixel = %d\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);  
    printf("So the screensize = %d(byte), using %d frame\n", screensize, finfo.smem_len/screensize);
    printf("vinfo.xoffset = %d, vinfo.yoffset = %d\n", vinfo.xoffset, vinfo.yoffset);  
    printf("vinfo.vmode is :%d\n", vinfo.vmode);  
    printf("finfo.ypanstep is :%d\n", finfo.ypanstep);  
    printf("vinfo.red.offset=0x%x\n", vinfo.red.offset);
    printf("vinfo.red.length=0x%x\n", vinfo.red.length);
    printf("vinfo.green.offset=0x%x\n", vinfo.green.offset);
    printf("vinfo.green.length=0x%x\n", vinfo.green.length);
    printf("vinfo.blue.offset=0x%x\n", vinfo.blue.offset);
    printf("vinfo.blue.length=0x%x\n", vinfo.blue.length);
    printf("vinfo.transp.offset=0x%x\n", vinfo.transp.offset);
    printf("vinfo.transp.length=0x%x\n", vinfo.transp.length);
    

    fbp =(char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp,0);  
    if ((int)fbp == -1)  
    {    
        printf ("Error: failed to map framebuffer device to memory.fbp = 0x%x\r\n" , fbp);  
				fprintf(stderr, "mmap failed with error: %d\n", errno);
        exit (4);  
    }
    printf("Get virt mem = %p\n", fbp);  


    pix_size = vinfo.xres * vinfo.yres;
    /* using first frame, for FBIOPAN_DISPLAY
     * 当刷新需要调用FBIOPAN_DISPLAY， 要告知驱动刷哪块帧， 用到下面两个参数
     * 如果使用第二帧buffer -> vinfo.xoffset = 0; vinfo.yoffset = vinfo.yres;
     */
    vinfo.xoffset = 0;
    vinfo.yoffset = 0;
		fbp[0] = 1;
		//memset(fbp , 0 , screensize);

    /* show color loop */
    
    #if 0
    while(num--) {
        printf("\ndrawing YELLOW......\n");
        fill_color16((short *)fbp, YELLOW, pix_size);
        //ioctl(fp, FBIOPAN_DISPLAY, &vinfo);
        sleep(3);

        printf("\ndrawing BLUE......\n");
        fill_color16((short *)fbp, BLUE, pix_size);
        //ioctl(fp, FBIOPAN_DISPLAY, &vinfo);
        sleep(3);
        
        printf("\ndrawing RED......\n");
        fill_color16((short *)fbp, RED, pix_size);
        //ioctl(fp, FBIOPAN_DISPLAY, &vinfo);
        sleep(3);
    }
    
    #endif
#if 0
    /*这是你想画的点的位置坐标,(0，0)点在屏幕左上角*/    
    x = 10;  
    y = 10;  
    location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;  
    test_fbp = fbp + location;
    printf("draw line.......\n");
    
    #if 0
    for(i = 0; i < (vinfo.xres - x); i++)
        *test_fbp++ = 0xffff0000;

    for(i = 0 ; i < 100 ; i++)
    {
    	y++;
    	location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;  
    	test_fbp = fbp + location;
    	*test_fbp = 0xffff0000;
    }

    x = 15;  
    y = 15;  
    location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;  
    test_fbp = fbp + location;
    printf("draw line.......\n");
    for(i = 0; i < (vinfo.xres - x); i++)
        *test_fbp++ = 0xff0000ff;

    for(i = 0 ; i < 100 ; i++)
    {
    	y++;
    	location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;  
    	test_fbp = fbp + location;
    	*test_fbp = 0xff0000ff;
    }
    #endif
    
	test_draw_line(fbp , 10 , 300 , 10 , 0xffff0000);
	//test_draw_line(fbp , 200 , 300 , 100 , 0xff00ff00);
	//test_draw_line(fbp , 400 , 300 , 100 , 0xff0000ff);
	
	//ioctl(fp, FBIOPAN_DISPLAY, &vinfo);
#endif
	for(int i = 0 ; i < 10 ; i++)
	{
		*fbp++ = i;
		printf("fbp[%d] = 0x%x\r\n" , i , ((unsigned int*)fbp)[i]);
	}
	munmap(fbp, screensize); /*解除映射*/  

	close (fp);
	return 0;
}
