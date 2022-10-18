# ucas_rootkit
## 项目环境
操作系统：Ubuntu 20.04 LTS
Linux内核版本：5.15.0-50-generic
## 项目文件

- Makefile
- rootkit.h
   - DEVICE_NAME是insmod后在/dev中的名称
- rootkit.c
   - rootkit_init，初始化时执行，流程为：
      1. 注册设备
      2. 创建device class
      3. 创建device inode
      4. 打开device，更改i_mode=0666，使得用户态可以写入
      5. /proc/modules 信息隐藏
      6. /sys/module/ 信息隐藏
   - rootkit_exit
- functions.c
   - 自定义的open、read、write、release和ioctl函数，应用于file_operations结构体
      - write根据输入内容实现各种功能，具体见使用方法
      - ioctl实现进程间通信，功能包括：
         - 0x1001，隐藏进程
         - 0x1002，恢复隐藏进程
   - hidefile，隐藏文件
- hide_itself.c
   - 打开/dev/intel_rapl_msrdv后，调用0x1001命令，隐藏本进程
- reshow.c
   - 打开/dev/intel_rapl_msrdv后，调用0x1002命令，恢复隐藏的进程，传入的为进程编号
## 功能实现
### 模块隐藏
lsmod指令&/proc/modules文件，通过从__this_module.list双向链表中脱链实现
/sys/module/，通过调用kobject_del实现
### 文件隐藏
隐藏.ko文件和/dev/设备名，使用ls和find指令，通过将文件的dentry从d_child中脱链实现，目前存在一些问题：
> 不能隐藏：
> ![image.png](https://cdn.nlark.com/yuque/0/2022/png/26116274/1665806152966-58ccd8c6-d07b-4617-9ea4-a4faa46e45df.png#clientId=u0fc3b868-a8fe-4&crop=0&crop=0&crop=1&crop=1&errorMessage=unknown%20error&from=paste&height=18&id=yVaL2&margin=%5Bobject%20Object%5D&name=image.png&originHeight=23&originWidth=521&originalType=binary&ratio=1&rotation=0&showTitle=false&size=3820&status=error&style=none&taskId=ua38697aa-d1a9-495b-bdf1-09be2b8b0d1&title=&width=416.8)
> 可以隐藏：
> ![image.png](https://cdn.nlark.com/yuque/0/2022/png/26116274/1665806199471-966fb6e9-1c86-4acd-b7d8-91512b0ccc39.png#clientId=u0fc3b868-a8fe-4&crop=0&crop=0&crop=1&crop=1&errorMessage=unknown%20error&from=paste&height=17&id=wbTZI&margin=%5Bobject%20Object%5D&name=image.png&originHeight=21&originWidth=627&originalType=binary&ratio=1&rotation=0&showTitle=false&size=5331&status=error&style=none&taskId=u7f9380bb-9cb1-4138-af02-ce0bbb224f6&title=&width=501.6)![image.png](https://cdn.nlark.com/yuque/0/2022/png/26116274/1665810622582-3f1b5d70-5a28-473b-8293-f1e95adf4cc9.png#clientId=u0fc3b868-a8fe-4&crop=0&crop=0&crop=1&crop=1&errorMessage=unknown%20error&from=paste&height=18&id=vC5Fb&margin=%5Bobject%20Object%5D&name=image.png&originHeight=23&originWidth=546&originalType=binary&ratio=1&rotation=0&showTitle=false&size=3998&status=error&style=none&taskId=u9023eb88-1e08-4de0-bca6-8ea63b0381c&title=&width=436.8)

### 进程隐藏与恢复
遍历/proc/pid，或使用 ps 指令，均无法找到 hide_itself 进程。通过从 task_struct 链表与 pid 链表中摘除进程实现。
## 使用方法
```bash
make  # 编译后获得rootkit.ko
sudo insmod rootkit.ko  # 装载rootkit模块

echo 'hide xxx' > /dev/intel_rapl_msrdv  # 隐藏文件xxx
echo 'root' > /dev/intel_rapl_msrdv  # id和组id都提权为r0级

./hide_itself &  # 执行用户态进程，进行自我隐藏
./reshow 0  # 恢复0号隐藏进程，即hide_itself

sudo rmmod rootkit  # 卸载rootkit
```
## 参考
[【OS.0x01】Linux Kernel II：内核简易食用指北](https://arttnba3.cn/2021/02/21/OS-0X01-LINUX-KERNEL-PART-II/#0x04-%E7%BC%96%E5%86%99%E5%8F%AF%E8%A3%85%E8%BD%BD%E5%86%85%E6%A0%B8%E6%A8%A1%E5%9D%97%EF%BC%88LKMs%EF%BC%89)
[【CODE.0x01】简易 Linux Rootkit 编写入门指北](https://arttnba3.cn/2021/07/07/CODE-0X01-ROOTKIT)
