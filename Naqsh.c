#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#ifdef HAVE_LIBSWSCALE
#include <libswscale/swscale.h>
#endif
#include <time.h>
#include <string.h>

struct Naqsh
{
    XImage *image;
    size_t imageSize;
    unsigned char* imageData;
    int height;
    int width;
    int dstStride ;
    int channel ;

} Naqsh;

void torgb(struct Naqsh * Naqsh)
{

    int srcStride = Naqsh->image->bytes_per_line;
    int dstStride = Naqsh->width * 3;  // RGB pixel data stride
    for (int y = 0; y < Naqsh->height; y++) {
        unsigned char* srcRow = (unsigned char*)(Naqsh->image->data) + (y * srcStride);  // RGBA row
        unsigned char* dstRow = Naqsh->imageData + (y * dstStride);  // RGB row
        for (int x = 0; x < Naqsh->width; x++) {
            unsigned char* srcPixel = srcRow + (x * 4);  // RGBA pixel
            unsigned char* dstPixel = dstRow + (x * 3);  // RGB pixel

            dstPixel[0] = srcPixel[2];  // Red
            dstPixel[1] = srcPixel[1];  // Green
            dstPixel[2] = srcPixel[0];  // Blue
        }
    }
    Naqsh->dstStride = dstStride;
}

void torgbSws(struct Naqsh *Naqsh)
{ 
#ifdef HAVE_LIBSWSCALE
    int imageWidth = Naqsh->image->width;
    int imageHeight = Naqsh->image->height;
    struct SwsContext* swsContext = sws_getContext(imageWidth, imageHeight, AV_PIX_FMT_BGRA, imageWidth, imageHeight, AV_PIX_FMT_RGB24, 0, NULL, NULL, NULL);

    // Perform color space conversion and scaling
    const uint8_t* inData[1] = { (const uint8_t*)Naqsh->image->data };
    int inLinesize[1] = { Naqsh->image->bytes_per_line };
    int outLinesize[1] = { Naqsh->image->width * Naqsh->channel };
    sws_scale(swsContext, inData, inLinesize, 0, imageHeight, &Naqsh->imageData, outLinesize);
#else
    torgb(Naqsh);
#endif

}
int main()
{
    Display* display = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(display);
    // Get the screen dimensions
    Naqsh.width = DisplayWidth(display, DefaultScreen(display));
    Naqsh.height = DisplayHeight(display, DefaultScreen(display));
    // Create an XImage with the screen dimensions
    Naqsh.image = XGetImage(display, root, 0, 0, Naqsh.width, Naqsh.height, AllPlanes, ZPixmap);
    Naqsh.channel = 3; 
    if (Naqsh.image->depth == 32 || Naqsh.image->depth == 24) { //rgba
        Naqsh.imageSize = Naqsh.image->width * Naqsh.image->height * Naqsh.channel;
        Naqsh.imageData = (unsigned char*)malloc(Naqsh.imageSize);
        if (!Naqsh.imageData) {
            // Handle memory allocation error
            return 1;
        }
        torgbSws(&Naqsh);
        // Save RGB pixel data as PNG
        time_t rawTime;
        time(&rawTime);
        struct tm* timeInfo = localtime(&rawTime);
        char fileName[256];
        strftime(fileName, sizeof(fileName), "Naqsh_%Y%m%d%H%M%S.png", timeInfo);

        // Construct the full path to the output file
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", getenv("HOME"), fileName);

        int secces = stbi_write_png(fullPath, Naqsh.image->width, Naqsh.image->height, Naqsh.channel, Naqsh.imageData, Naqsh.dstStride);
        if(secces == 1){
            printf("%s \n",fullPath);
        }
        free(Naqsh.imageData);
    } else {
        printf("Failed\n");
        return 1;
    }
    // Close the X display connection
    XCloseDisplay(display);

    return 0;
}

