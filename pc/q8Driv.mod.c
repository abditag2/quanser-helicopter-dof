#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8b54bed4, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x2feb31d9, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0xdfae6b80, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x696e0c2, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0x4c4fef19, __VMLINUX_SYMBOL_STR(kernel_stack) },
	{ 0xbc8b4ed1, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xed0b11db, __VMLINUX_SYMBOL_STR(no_llseek) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x43733505, __VMLINUX_SYMBOL_STR(nonseekable_open) },
	{ 0x8b18496f, __VMLINUX_SYMBOL_STR(__copy_to_user_ll) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
	{ 0x2072ee9b, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x9cb23bde, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x5edfcde5, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0xfe8a7e7a, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x922256f8, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x971e5df, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000011E3d00000010sv00005155sd000002ACbc*sc*i*");

MODULE_INFO(srcversion, "DB2C3148080B0C90795288E");
