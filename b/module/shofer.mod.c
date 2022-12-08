#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0xf704969, "module_layout" },
	{ 0xf247fba3, "param_ops_int" },
	{ 0x24d273d1, "add_timer" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x4578f528, "__kfifo_to_user" },
	{ 0x30a80826, "__kfifo_from_user" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xc946dda0, "cdev_del" },
	{ 0xc378cea7, "cdev_add" },
	{ 0x2d725fd4, "cdev_init" },
	{ 0x7c797b6, "kmem_cache_alloc_trace" },
	{ 0xd731cdd9, "kmalloc_caches" },
	{ 0x37a0cba, "kfree" },
	{ 0xbd462b55, "__kfifo_init" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xf23fcb99, "__kfifo_in" },
	{ 0x13d0adf7, "__kfifo_out" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0x281823c5, "__kfifo_out_peek" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x4c55f5ef, "pv_ops" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "CB3A8761A22C331107D9841");
