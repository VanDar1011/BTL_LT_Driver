#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xdd8f8694, "module_layout" },
	{ 0x867847b8, "usb_deregister" },
	{ 0x785cf894, "usb_register_driver" },
	{ 0xa2e511ef, "vfs_write" },
	{ 0x5e5292c, "filp_close" },
	{ 0x37a0cba, "kfree" },
	{ 0x908d38d9, "vfs_read" },
	{ 0x56b1771b, "current_task" },
	{ 0xca7a3159, "kmem_cache_alloc_trace" },
	{ 0x428db41d, "kmalloc_caches" },
	{ 0xddd346a3, "filp_open" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xc4952f09, "cdev_add" },
	{ 0x2064fa56, "cdev_init" },
	{ 0x7749276a, "device_create" },
	{ 0x2871e975, "__class_create" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x56470118, "__warn_printk" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xb65e5a32, "class_destroy" },
	{ 0x22e92418, "device_destroy" },
	{ 0x22b90774, "cdev_del" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v21C4p0CD1d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "D9D046AECDA32049A3A8FF4");
