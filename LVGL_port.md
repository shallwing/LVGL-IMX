本文介绍嵌入式轻量化图形库**LVGL 8.2**移植到Linux开发板IMX6ULL的步骤。

[TOC]

# LVGL简介

LVGL最初是由匈牙利人Gabor Kiss-Vamosi所创建的，目前的更新到了稳定版本V8.3，**本文档所移植的版本是V8.2**。LVGL的官方文档和Github源码链接如下：

[LVGL - Light and Versatile Embedded Graphics Library](https://github.com/lvgl/lvgl.git)

[LVGL in Github](https://github.com/lvgl/lvgl)

**LVGL**是"Light and Versatile Graphics Library"的简称（早年称之为"LittleVGL"，后改名为此），叫做”轻量级多功能图形界面库“，是一种适用于大多数嵌入式设备的图形化界面库。与QT类似，LVGL借用了面向对象的编程思想，但使用的编程语言是C，这使得LVGL编程易于理解与上手。



# 移植LVGL的硬件条件

大多数嵌入式主板都可以支持LVGL（包括单片机、Arduino、以及Linux开发板等），但是**处理器的位数必须是16位及以上**。



# 移植准备

## 1. 源码下载

本文档针对LVGL 8.2版本在Linux IMX6ULL开发板上移植LVGL需要下载的源码如下：

- lvgl：https://github.com/lvgl/lvgl.git
- lv_drivers：https://github.com/lvgl/lv_drivers.git
- lv_port_linux_frame_buffer：https://github.com/lvgl/lv_port_linux_frame_buffer.git

lvgl：包含了LVGL基本的源码，以及官方给出的LVGL demo；

lv_drivers：包含了大多数设备的显示控制器和触摸驱动程序，主要用来指定显示屏使用哪一种驱动框架（包括FB、DRM等驱动程序框架）；

lv_port_linux_frame_buffer：**主函数文件所在的目录**，整个工程的主文件夹，lvgl和lv_drivers都应放在此目录下。

可以在具有代理服务器的情况下克隆上述三个仓库的源码：

```
git clone -b release/v8.2 https://github.com/lvgl/lv_port_linux_frame_buffer.git
```

```
git clone -b release/v8.2 https://github.com/lvgl/lvgl.git
```

```
git clone -b release/v8.2 https://github.com/lvgl/lv_drivers.git
```

## 2. 驱动加载

**在使用本文档的教程之前，请确保IMX6ULL已加载FB或DRM驱动**。



# 移植过程

## 源码修改

先将下载好的源码文件夹lvgl和lv_drivers放在lv_port_linux_frame_buffer的路径下；

```php+HTML
shallwing@9d57f9229b66:~/lv_port_linux_frame_buffer$ ls
LICENSE  lv_conf.h  lv_drivers/  lv_drv_conf.h  lvgl/  main.c  Makefile  mouse_cursor_icon.c  README.md
```

下面将对源码中的Makefile、lv_conf.h、main.c、lv_drv_conf.h等文件进行修改，以达到让编译后的程序成功在IMX6ULL上运行的目标。

### 1. 修改lv_conf.h

打开lv_conf.h，先看到第15行，检查文件是否使能：

```C
/* clang-format off */
#if 1 /*Set it to "1" to enable content*/
```

使能该文件，应该将#if后面的0改为1。

之后看到第27行，将宏**LV_COLOR_DEPTH设置为16**：

```C
/*Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define LV_COLOR_DEPTH 16
```

这里的宏LV_COLOR_DEPTH表示的是显示屏的颜色深度，由于**我们使用的LCD屏幕是RGB565格式的**（即表示RGB三个颜色通道分别用5bytes、6bytes、5bytes的空间来存储），所以色深应该设置为16（bytes）。

然后看到48至67行的代码片段：

```C
/*1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()`*/
#define LV_MEM_CUSTOM 1
#if LV_MEM_CUSTOM == 0
    /*Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB)*/
    #define LV_MEM_SIZE (2 * 1024U * 1024U)          /*[bytes]*/

    /*Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too.*/
    #define LV_MEM_ADR 0     /*0: unused*/
    /*Instead of an address give a memory allocator that will be called to get a memory pool for LVGL. E.g. my_malloc*/
    #if LV_MEM_ADR == 0
        //#define LV_MEM_POOL_INCLUDE your_alloc_library  /* Uncomment if using an external allocator*/
        //#define LV_MEM_POOL_ALLOC   your_alloc          /* Uncomment if using an external allocator*/
    #endif

#else       /*LV_MEM_CUSTOM*/
    #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /*Header for the dynamic memory function*/
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif     /*LV_MEM_CUSTOM*/
```

**这一段代码是用来进行显存配置的，将LV_MEM_CUSTOM设置为1，则表示使能显存分配**。在开启显存分配之后，系统便可以给LCD屏分配运行显存。

看到第80行至84行的代码：

```C
/*Default display refresh period. LVG will redraw changed areas with this period time*/
#define LV_DISP_DEF_REFR_PERIOD 10      /*[ms]*/

/*Input device read period in milliseconds*/
#define LV_INDEV_DEF_READ_PERIOD 10     /*[ms]*/
```

这里设置的是屏幕的刷新时间，单位是毫秒(ms)。我们将其中的30ms改为10ms。

看到86至92行的代码：

```C
/*Use a custom tick source that tells the elapsed time in milliseconds.
 *It removes the need to manually update the tick with `lv_tick_inc()`)*/
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE <stdint.h>         /*Header for the system time function*/
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (custom_tick_get())    /*Expression evaluating to current system time in ms*/
#endif   /*LV_TICK_CUSTOM*/
```

这里设置的是心跳时间，在主函数文件main.c中有一个custom_tick_get的函数，**用于之后的事件响应编程和定时任务编程，若此功能没有使能，则点击屏幕上的组件将没有响应**。

看到第671行，为了看到移植的效果，我们先使能官方的demo，来检测是否移植成功：

```C
/*Show some widget. It might be required to increase `LV_MEM_SIZE` */
#define LV_USE_DEMO_WIDGETS        1
#if LV_USE_DEMO_WIDGETS
#define LV_DEMO_WIDGETS_SLIDESHOW  0
#endif
```

### 2. 修改lv_drv_conf.h

此文件用于配置显示屏所使用的底层驱动，我们使用DRM驱动框架来点亮LCD屏，所以对于此文件的修改，一律是屏蔽FBDEV而使能DRM。

看到第11行，先使能此文件，将“#if 0”改为“#if 1”：

```C
/* clang-format off */
#if 1 /*Set it to "1" to enable the content*/
```

看到第318行，屏蔽FBDEV的驱动，将宏USE_FBDEV改为0：

```C
/*-----------------------------------------
 *  Linux frame buffer device (/dev/fbx)
 *-----------------------------------------*/
#ifndef USE_FBDEV
#  define USE_FBDEV           0
#endif

#if USE_FBDEV
#  define FBDEV_PATH          "/dev/fb0"
#endif
```

然后看到337行，使能DRM驱动，将USE_DRM改为1：

```C
/*-----------------------------------------
 *  DRM/KMS device (/dev/dri/cardX)
 *-----------------------------------------*/
#ifndef USE_DRM
#  define USE_DRM           1
#endif

#if USE_DRM
#  define DRM_CARD          "/dev/dri/card0"
#  define DRM_CONNECTOR_ID  -1	/* -1 for the first connected one */
#endif
```

看到第441行，使能鼠标或者触摸板作为evdev界面，将USE_EVDEV设置为1：

```C
#ifndef USE_EVDEV
#  define USE_EVDEV           1
#endif

#ifndef USE_BSD_EVDEV
#  define USE_BSD_EVDEV       0
#endif

#if USE_EVDEV || USE_BSD_EVDEV
#  define EVDEV_NAME   "/dev/input/event1"        /*You can use the "evtest" Linux tool to get the list of devices and test them*/
#  define EVDEV_SWAP_AXES         0               /*Swap the x and y axes of the touchscreen*/

#  define EVDEV_CALIBRATE         1               /*Scale and offset the touchscreen coordinates by using maximum and minimum values for each axis*/

#  if EVDEV_CALIBRATE
#    define EVDEV_HOR_MIN         0               /*to invert axis swap EVDEV_XXX_MIN by EVDEV_XXX_MAX*/
#    define EVDEV_HOR_MAX      800               /*"evtest" Linux tool can help to get the correct calibraion values>*/
#    define EVDEV_VER_MIN         0
#    define EVDEV_VER_MAX      480
#  endif  /*EVDEV_CALIBRATE*/
#endif  /*USE_EVDEV*/
```

除此之外，还需要指定evdev设备节点的路径，一般来说，evdev输入设备节点的路径在/dev/input下，对应于event文件，然而/dev/input下有可能有多个event文件，此时我们可以用hexdump命令来检测：

```C
hexdump event1
```

运行上面的命令之后，再点击LCD屏一下，如果发现终端输出了一大堆十六进制数，则说明LCD的evdev的输入设备节点是它，否则，就换一个event文件进行测试。然后，需要就自己显示屏的分辨率来设置其中的EVDEV_HOR_MAX和EVDEV_VER_MAX，我们的LCD显示屏分辨率是800*480，所以两个宏分别设置为800和480。

**evdev是输入设备的配置，只有使能了evdev，触摸屏幕才会有反应**。

### 3. 修改main.c文件

在main.c文件里面，我们主要修改其中包含的头文件、使用的驱动类型，以及demo函数等。

先看到文件开头包含的头文件，修改其中包含的驱动头文件，将第3行的"fbdev.h"改为"drm.h"：

```C
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/drm.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
```

随后，看到第10行，修改显示缓冲区的大小，即800*480，分辨率的大小：

```C
#define DISP_BUF_SIZE (800 * 480)
```

看到第18行，修改驱动设备的初始化函数，将fbdev_init()改为drm_init()：

```C
/*Linux frame buffer device init*/
drm_init();
```

看到第27至34行的代码部分，这一段是初始化和设置显示驱动的部分：

```C
/*Initialize and register a display driver*/
static lv_disp_drv_t disp_drv;
lv_disp_drv_init(&disp_drv);
disp_drv.draw_buf   = &disp_buf;
disp_drv.flush_cb   = fbdev_flush;
disp_drv.hor_res    = 800;
disp_drv.ver_res    = 480;
lv_disp_drv_register(&disp_drv);
```

照着我们的实际情况，进行修改与适配，由于LCD屏使用DRM驱动框架，所以将其中的flush_cb由fbdev_flush改为drm_flush，将其中的hor_res改为800，ver_res改为480，其他不改变。

看到代码的第46至50行，我们不使用鼠标作为LCD显示屏的输入设备，所以mouse部分将它注释掉：

```C
#if 0
    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/
#endif
```

看到第54行，这里面告诉了我们搭建整个LVGL应用程序的函数为lv_demo_widgets，通过查看main.c的头文件就可以知道，这个函数在“lvgl/demos/widgets/lv_demo_widgets.c”中定义。

```C
/*Create a Demo*/
lv_demo_widgets();
```

### 4. 修改Makefile文件

打开主文件夹下的Makefile文件，对其进行修改。

看到第4行，将CC编译器修改为自己的交叉编译器，修改如下：

```
CC = /opt/buildroot/cortexA7/bin/arm-buildroot-linux-gnueabihf-gcc
```

注释掉第20行，使其不能编译鼠标输入设备的源码：

```
# CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c 
```

## 源码编译

源码修改完成之后，直接运行下面的命令进行编译：

```
make -j48
```

源码编译中会产生错误：

```
/home/shallwing/lv_port_linux_frame_buffer/lv_drivers/display/drm.c:28:10: 致命错误：drm_fourcc.h：No such file or directory
   28 | #include <drm_fourcc.h>
      |          ^~~~~~~~~~~~~~
编译中断。
make: *** [Makefile:35: /home/shallwing/lv_port_linux_frame_buffer/lv_drivers/display/drm.o] Error 1
```

这种错误的原因是没有找到/opt/buildroot下的头文件drm_fourcc.h，通过在/opt/buildroot下用find命令查找此文件，可以得到它的路径为：

```
./cortexA7/arm-buildroot-linux-gnueabihf/sysroot/usr/include/drm/drm_fourcc.h
```

所以文件”lv_drivers/display/drm.c“中应该改为：

```C
#include <drm/drm_fourcc.h>
```

接着再次编译，又报出了"undefined reference"的错误：

```
drm.c:(.text+0x6c): undefined reference to `drmIoctl'
/opt/buildroot/cortexA7/lib/gcc/arm-buildroot-linux-gnueabihf/9.4.0/../../../..
```

这是因为系统在编译源码的时候，不会自动加载drm相关的链接器，需要我们自己加载，我们只需要在主Makefile中添加即可，在第8行和第9行之间添加：

```
LDFLAGS += -ldrm
```

接下来编译，虽然不会报错，但是会抛出“隐式声明函数custom_tick_get”或变量未使用的警告，这对后面的结果演示影响不大。在编译完成之后，源码主目录下会出现可执行文件demo，这便是最终生成的应用程序。



# 效果演示

在开发板上运行./demo的程序之后，会报出打印配置信息，并且屏幕上会显示官方的demo界面，说明移植成功了：

```
root@igkboard:~# ./demo 
drm: Found plane_id: 31 connector_id: 35 crtc_id: 33
drm: 800x480 (0mm X 0mm) pixel format RG16
DRM subsystem and buffer mapped successfully
```



# 参考文档

[IMX6ULL移植LVGL](https://blog.csdn.net/qq_51569769/article/details/126445295)

[LVGL官方源码](https://github.com/lvgl/lvgl.git)