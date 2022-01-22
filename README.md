# Build a lightweight webbox on Intel NUC and Raspberrypi

This project is about building a lightweight "webbox", which you can use for simple start of your own Internet appliance.

Webbox is based on Yocto/Linux so it's completely customisable and runs on any compatible hardware, such as Intel NUC and Raspberrypi.

Webbox has Linux and builtin Chromium browser so it's good for both native application and web (such as HTML5 games) development.

## Quick start

In the picture is my webbox running on Intel NUC.

![Webbox](img/webbox.jpg)

Here is a prebuilt image for Intel NUC. See below how to get the image in your device.
- Intel NUC image (550MB) https://minimindgames.com/webbox/webbox-image-intel-corei7-64.wic.bz2

> You may need to update `date -s "2022..."` if your device has no time to handle https-pages.

## Contents

- [Setup build](#setup-build)
- [Build Qt test app](#build-qt-test-app)
- [Test on qemu](#test-on-qemu)
- [Test on Intel NUC](#test-on-intel-nuc)
- [Test on Raspberrypi](#test-on-raspberrypi)
- [Chromium browser](#chromium-browser)
- [Make it yours](#make-it-yours)
- [Gamepad controller](#gamepad-controller)
- [Webbox image](#webbox-image)
- [Start application](#start-application)

## Setup build

Setup Yocto build for poky, see Yocto Quick Build https://docs.yoctoproject.org/brief-yoctoprojectqs/index.html
```
~/$ sudo apt install gawk wget git diffstat unzip texinfo gcc build-essential chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev pylint3 xterm python3-subunit mesa-common-dev zstd liblz4-tool

~/$ git clone -b honister git://git.yoctoproject.org/poky
~/$ cd poky
```

Build weston image, see images explained at https://docs.yoctoproject.org/ref-manual/images.html (shortly: weston is here for wayland referencce implementation and its's about to replace old X11 based GUI).
```
# By default machine is qemux86-64 
~/poky$ source oe-init-build-env
~/poky/build$ time bitbake core-image-weston
```

## Build Qt test app

> You may be interested to use Qt's repo [Boot to Qt](https://doc.qt.io/QtForDeviceCreation/b2qt-index.html). I'm doing this manually just to dig in to more details.

Download Qt and our Qt test applications (ie. this project)
```
~/poky/build$ git clone git://code.qt.io/yocto/meta-qt6.git ../meta-qt6/
~/poky/build$ bitbake-layers add-layer ../meta-qt6
~/poky/build$ git clone git@github.com:minimindgames/meta-webbox.git ../meta-webbox/
~/poky/build$ bitbake-layers add-layer ../meta-webbox
```

Add Qt test app named as `qapp` in `~/poky/build/conf/local.conf`.
```
# qapp for Qt-based GUI app.
DISTRO_FEATURES:append = " wayland"
#IMAGE_INSTALL:append = " qtwayland" => added with "wayland" feature by "packagegroup-qt6-addons.bb"
IMAGE_INSTALL:append = " qapp"
```

Our image is based on core-image-weston so features should be good but let's check, see https://www.yoctoproject.org/docs/current/mega-manual/mega-manual.html#dev-using-wayland-and-weston
```
# check that we have "wayland" and "weston", then build
~/poky/build$ time bitbake core-image-weston -e |grep ^DISTRO_FEATURES=
```

Build qapp to see it's good and then compete image.
```
~/poky/build$ bitbake qapp
~/poky/build$ bitbake core-image-weston
```

## Test on qemu

Check that qapp is runnable (`nographic` is good to make it faster and on "putty" terminal)
```
~/poky/build$ runqemu tmp/deploy/images/qemux86-64/core-image-weston-qemux86-64.qemuboot.conf
	qemux86-64 login: root
		open weston-terminal and type "qcmd"
			see "Hello from Qcmd" on terminal
				Note: you can exit qemu with "c-a x"
```

![qapp screen capture](img/qemu-qapp.jpg)

## Test on Intel NUC

> I tried before doing any of this and Ubuntu 20.04 image works fine on NUC, so my backup plan is to use Ubuntu, if this Yocto experiment will be a dead end.

Download support for Intel
```
~/poky$ git clone git://git.yoctoproject.org/meta-intel
~/poky/build$ bitbake-layers add-layer ../meta-intel
```

Build for i7-machine. Actually, my NUC has i3 but I'm feeling lucky.
```
~/poky/build$ export MACHINE=raspberrypi3 # or eg. echo 'MACHINE = "intel-corei7-64"' >> conf/local.conf
~/poky/build$ time bitbake core-image-weston
```

Flash the image on USB stick. Note that it will block for a few minutes. I noticed that writing is more reliable if `sync` until "Dirty" is zeroed after inserting USB (before issuing `dd`).
```
~/poky/build$ dmesg -w # insert USB stick to find your /dev/sdX for command below then break ctrl-c
~/poky/build$ sudo dd if=tmp/deploy/images/intel-corei7-64/core-image-sato-dev-intel-corei7-64.wic of=/dev/sdc bs=1M status=progress conv=fsync
```

Eject USB stick from PC and insert it to NUC. To boot NUC from USB use F2 to enter BIOS to change boot order.

Open `weston-terminal` and type `qapp` to see our all mighty test app again (previosly with qemu).

![NUC qapp](img/nuc-qapp.jpg)

Now you can dd image from USB stick to internal drive so NUC can boot without USB.

> This will wipe out anything on your NUC, skip if unsure.

```
$ dd if=/dev/sdb of=/dev/sda bs=1M
```

I built also `core-image-sato-dev` to see if Yocto's "mobile" look-and-feel would serve my web gaming interests, see the picture below.

![NUC sato](img/nuc-sato.jpg)
 
It looks quite useful out-of-the-box, and it has even some preinstalled games so you can start playing immediately.
 
## Test on Raspberrypi

Download Raspberrypi support with its OE dependencies, see https://meta-raspberrypi.readthedocs.io/en/latest/readme.html
```
~/poky$  git clone -b honister git@github.com:openembedded/meta-openembedded.git
~/poky/build$ bitbake-layers add-layer ../meta-openembedded/meta-oe
~/poky/build$ bitbake-layers add-layer ../meta-openembedded/meta-python
~/poky/build$ bitbake-layers add-layer ../meta-openembedded/meta-multimedia
~/poky/build$ bitbake-layers add-layer ../meta-openembedded/meta-networking

~/poky$ git clone -b honister git://git.yoctoproject.org/meta-raspberrypi
~/poky/build$ bitbake-layers add-layer ../meta-raspberrypi
```

Add features and installs in `build/conf/local.conf`
```
DISTRO_FEATURES:append = " wayland"
CORE_IMAGE_EXTRA_INSTALL = "wayland weston"
```

I have only 200GB space so I removed `~/poky/build/tmp` before building Raspberrypi.
```
~/poky/build$ export MACHINE=raspberrypi3 # or edit conf/local.conf
~/poky/build$ time bitbake core-image-weston
```

Now we can flash the wic-image to SD card (I use Win32DiskImager) and boot Raspberrypi with it.

> The link from `core-image-weston-raspberrypi3.wic.bz2` to actual file was not working so I had to use target rootfs-file of symbolic link, and also with force `bzip2 -d -f` - don't ask why.

![Raspberrypi](img/raspberrypi-qapp.jpg)

With Raspberrypi in the picture, it's really starting to look like an embedded platform. As a side note, I checked also that sato-image was working on my Raspberrypi3.

## Chromium browser

Download chromium (and clang for it) then add to our build.
```
~/poky/build$ git clone -b honister git://github.com/kraj/meta-clang
~/poky/build$ bitbake-layers add-layer ../meta-clang
~/poky/build$ git clone git@github.com:OSSystems/meta-browser.git
~/poky/build$ bitbake-layers add-layer ../meta-browser/meta-chromium
```

Add chromium to `conf/local.conf`, obviously "wayland" version.
```
IMAGE_INSTALL:append = " chromium-ozone-wayland"
```

We want Chromium to `use-egl` and that's already defined in meta-webbox, let's check that's good for our build.
```
bitbake-layers show-appends |grep chromium
```

Our bbappend for chromium was found and parsed. Good. Ready to build the image again - and now with Chromium in build it will take double time.
```
~/poky/build$ time bitbake core-image-weston
```

I was so surprised that it compiled without errors that let's check it really got into our rootfs.

```
bitbake core-image-weston -c devshell
# cd ../rootfs
# find -name chromium
./usr/lib/chromium
./usr/bin/chromium
# exit
```

Chromium is there! I'm surprised and delighted. Can't wait to try it on my NUC.

Let's flash and boot NUC from USB again.

![Chromium](img/nuc-chromium.jpg)

Awesome! That was easier than expected.

> I actually got some kernel panic due to filesystem (ext) failing first, but reflashing fixed. You probably avoid this by calling `sync` after inserting USB before `dd`.

## Start chromium as non-root user

Linux `init` system was good back in 80s, but nowadays `systemd` is used for better control of system services. Modify `build/conf/local.conf` to use systemd.
```
DISTRO_FEATURES:append = " systemd"
DISTRO_FEATURES_BACKFILL_CONSIDERED += "sysvinit"
VIRTUAL-RUNTIME_init_manager = "systemd"
VIRTUAL-RUNTIME_initscripts = "systemd-compat-units"
```

You may want to check with Qemu before flashing full image that systemd services are active.
```
~/poky/build$ MACHINE=qemux86-64 bitbake core-image-minimal
~/poky/build$ runqemu tmp/deploy/images/qemux86-64/core-image-minimal-qemux86-64.qemuboot.conf nographic

qemux86-64 login: root
# Check systemctl services are active, also `ps |grep init`, `/sbin/init` and `/proc/1/exe` shall point to systemd
root@qemux86-64:~# systemctl --type=service
```

After flashing and booting NUC, I was genuinely surprised and delighted! The shell prompt was changed from `sh-5.1#` to `sh-5.1$` so the root user problem was indeed solved so we can remove `--no-sandbox` from chromium startup scripts. And also, who wouldn't like to see logs with `journalctl` instead of `cat /var/log/`.
```
sh-5.1$ whoami
weston
```

## Start application

This is a web console (and not just another linux box) so let's start to web at boot.

This project has a `startapp` recipe to start chromium as a `systemd` service right after weston has started.

Add startapp to the image, like `CORE_IMAGE_EXTRA_INSTALL += "chromium-ozone-wayland startapp"`.

## Make it yours

You can now customize weston GUI for your needs simply by modifying `weston-init.bbappend` and `startapp.sh`.

By default, the build has my preferences:
- Launch icon to open Chromium web browser
- Launch icon for Linux terminal to dig deeper
- Dim screen when idle for 10 minutes
- Screen lock disabled

## Gamepad controller

A webbox shall have a gamepad for gaming, of course.

The Raspberrypi image already has gamepad supported, but it's a kernel module so let's load it (and ff-memless is needed also based on `dmesg`).
```
# insmod lib/modules/5.10.78-v7/kernel/drivers/input/ff-memless.ko
# insmod lib/modules/5.10.78-v7/kernel/drivers/input/joystic/xpad.ko
```

NUC's intel-kernel is not preconfigured with gamepad support, so let's enable it. For details, see Yocto's how to config kernel https://www.yoctoproject.org/docs/latest/kernel-dev/kernel-dev.html#configuring-the-kernel
```
~/poky/build$ bitbake linux-intel -c kernel_configme -f
~/poky/build$ bitbake linux-intel -c menuconfig
# press '/' to search for "joystic" => and we should find it at "drivers/input/joystic/xpad"
# Set <*> in Joystic interface (Raspberrypi had <M> for module so could fix that to <*> as well)
# Save and exit
~/poky/build$ bitbake linux-intel -c diffconfig
```

It created `fragment.cfg` which we want in our build so add it to your build, similar to `joystic.cfg` in meta-webbox, and check that kernel still builds.
```
~/poky/build$ bitbake-layers show-appends |grep linux-intel # just to check the linux-intel fragment for joystic applies
~/poky/build$ bitbake linux-intel -c cleansstate
~/poky/build$ bitbake linux-intel
```

Then build weston-image, flash it to NUC, plug in a gamepad and reboot NUC. Some of my web games support gamepad so they can be used to check that chromium has now gamepad, e.g. `# chromium --no-sandbox minimindgames.com/en/sudoku-for-kids/`.

![Game pad](img/gamepad.jpg)

In the picture, you can see my old gamepad for Playstation 3 and a green arrow indicating that "virtual stick" is up-right when left-stick of gamepad is pushed up-right.

> If your gamepad is not working, check the generated configuration if something could match to your gamepad. Path to config-file should be something like `~/poky/build/tmp/work/corei7-64-intel-common-poky-linux/linux-intel/5.15.1+gitAUTOINC+bee5d6a159_d7d7ea689c-r0/linux-corei7-64-intel-common-standard-build/.config`.

## Webbox image

Finally, it's kind of dirty to modify `local.conf` extensively, so I made an image which you can use.
```
~/poky/build$ time bitbake webbox-image
```

We should have something like `tmp/deploy/images/intel-corei7-64/webbox-image-intel-corei7-64.wic` which we can flash on NUC.

## Conclusion

We have now a Linux based webbox with Chromium browser.

Let's find and play some great HTML5 games, e.g. at https://itch.io/games/platform-web and https://minimindgames.com/webbox/.

## Tips and tricks

Yocto tips:
- GEMU is very slow - patience is a virtue
- Monitor you disk space with `df` to not run out of disk space (200GB is good for one MACHINE with chromium)
- Building image on my P710 (with 56 cores) takes a bit less than an hour from scratch

Linux tips:
- Images are big so `dd` takes time, `cat /proc/meminfo |grep Dirty` to see progress, `sync` frequently
- Best Linux tip ever is to searh history with `ctrl-r`

Bitbake tips:
- https://elinux.org/Bitbake_Cheat_Sheet
