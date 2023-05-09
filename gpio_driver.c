#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple gpio driver for reading a button");
static dev_t my_device_nr;
static struct class* my_class;
static struct cdev my_device;
#define DRIVER_NAME "my_gpio"
#define DRIVER_CLASS "MyModuleClass"
//main코드에서 사용할 read함수 정의
static ssize_t driver_read(struct file* File, char* user_buffer, size_t count, loff_t* offs) {
  int to_copy, not_copied, delta;
  char tmp;
  to_copy = min(count, sizeof(tmp));
  tmp = gpio_get_value(27) + '0';
  not_copied = copy_to_user(user_buffer, &tmp, to_copy);
  delta = to_copy - not_copied;
  return delta;
}
//디바이스 파일을 open할때 호출하는 함수를 정의한다.
static int driver_open(struct inode* device_file, struct file* instance) {
  printk("button - open was called!\n");
  return 0;
}
//디바이스 파일을 close할때 호출하는 함수를 정의한다.
static int driver_close(struct inode* device_file, struct file* instance) {
  printk("button - close was called!\n");
  return 0;
}
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .release = driver_close,
  .read = driver_read
};
//모듈이 커널에 로딩되었을때 호출되는 함수 정의
static int __init ModuleInit(void) {
  printk("Hello, Kernel!\n");
  //디바이스 nr 할당
  if (alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
    printk("Device Nr. could not be allocated!\n");
    return -1;
  }
  printk("read_write - Device Nr. Major: %d, Minor: %d was registered!\n", my_device_nr >> 20, my_device_nr && 0xfffff);
  //디바이스 클래스 생성
  if ((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
    printk("Device class can not e created!\n");
    goto ClassError;
  }
  //디바이스 파일 생성
  if (device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
    printk("Can not create device file!\n");
    goto FileError;
  }
  //디바이스 파일 초기화
  cdev_init(&my_device, &fops);
  //커널에 디바이스 등록
  if (cdev_add(&my_device, my_device_nr, 1) == -1) {
    printk("Registering of device to kernel failed!\n");
    goto AddError;
  }
  /* GPIO 27 init */
  if (gpio_request(27, "rpi-gpio-27")) {
    printk("Can not allocate GPIO 27\n");
    goto AddError;
  }
  /* Set GPIO 27 direction */
  if (gpio_direction_input(27)) { //gpio를 인풋모드로 변경
    printk("Can not set GPIO 27 to input!\n");
    goto Gpio27Error;
  }
  return 0;
  Gpio27Error:
  gpio_free(27);
  AddError:
  device_destroy(my_class, my_device_nr);
  FileError:
  class_destroy(my_class);
  ClassError:
  unregister_chrdev_region(my_device_nr, 1);
  return -1;
}
//모듈이 커널에서 제거될때 호출되는 함수 정의
static void __exit ModuleExit(void) {
  gpio_free(27);
  cdev_del(&my_device);
  device_destroy(my_class, my_device_nr);
  class_destroy(my_class);
  unregister_chrdev_region(my_device_nr, 1);
  printk("Goodbye, Kernel\n");
}
module_init(ModuleInit);
module_exit(ModuleExit);
