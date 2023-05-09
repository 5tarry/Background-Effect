#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#define DRIVER_NAME "my_led"
#define DRIVER_CLASS "MyModuleClass2"
MODULE_LICENSE("GPL");
static dev_t my_device_nr;
static struct class* my_class;
static struct cdev my_device;
volatile unsigned int* gpio;
//디바이스 파일을 open할때 호출하는 함수를 정의한다.
static int driver_open(struct inode* inode, struct file* file)
{
  printk("led_device - open was called!\n");
  return 0;
}
//디바이스 파일을 close할때 호출하는 함수를 정의한다.
static int driver_close(struct inode* inode, struct file* file) {
  printk("led_device - close was called!\n");
  return 0;
}
// main 코드에서 사용할 write 함수 정의
static ssize_t driver_write(struct file* file, const char* user_buffer, size_t count, loff_t* offs) {
  int to_copy, not_copied, delta;
  char value;
  to_copy = min(count, sizeof(value));
  not_copied = copy_from_user(&value, user_buffer, to_copy);
  switch (value) {
    case '0'://꽃받침 동작일때 gpio세팅
    gpio_set_value(22, 1);
    gpio_set_value(23, 0);
    gpio_set_value(24, 0);
    break;
    case '1'://하트 동작일때 gpio세팅
    gpio_set_value(22, 0);
    gpio_set_value(23, 1);
    gpio_set_value(24, 0);
    break;
    case '2'://슈퍼맨 동작일때 gpio세팅
    gpio_set_value(22, 0);
    gpio_set_value(23, 0);
    gpio_set_value(24, 1);
    break;
    case '3'://아무동작도 검출되지 않았을때 gpio세팅
    gpio_set_value(22, 0);
    gpio_set_value(23, 0);
    gpio_set_value(24, 0);
    break;
    default:
    printk("Invalid Inputl\n");
    break;
  }
  delta = to_copy - not_copied;
  return delta;
}
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .release = driver_close,
  .write = driver_write
};
//모듈이 커널에 로딩되었을때 호출되는 함수 정의
static int __init ModuleInit(void){
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
  /* GPIO 22 init */
  if (gpio_request(22, "rpi-gpio-22")) {
    printk("Can not allocate GPIO 22\n");
    goto AddError;
  }
  /* Set GPIO 22 direction */
  if (gpio_direction_output(22, 0)) { //gpio22를 아웃풋 모드로 변경
    printk("Can not set GPIO 22 to output!\n");
    goto Gpio22Error;
  }
  /* GPIO 23 init */
  if (gpio_request(23, "rpi-gpio-23")) {
    printk("Can not allocate GPIO 23\n");
    goto AddError;
  }
  /* Set GPIO 23 direction */
  if (gpio_direction_output(23, 0)) { //gpio23을 아웃풋 모드로 변경
    printk("Can not set GPIO 23 to output!\n");
    goto Gpio23Error;
  }
  /* GPIO 24 init */
  if (gpio_request(24, "rpi-gpio-24")) {
    printk("Can not allocate GPIO 24\n");
    goto AddError;
  }
  /* Set GPIO 24 direction */
  if (gpio_direction_output(24, 0)) { //gpio24를 아웃풋 모드로 변경
    printk("Can not set GPIO 24 to output!\n");
    goto Gpio24Error;
  }
  return 0;
  Gpio22Error:
  gpio_free(22);
  Gpio23Error:
  gpio_free(23);
  Gpio24Error:
  gpio_free(24);
  AddError:
  device_destroy(my_class, my_device_nr);
  FileError:
  class_destroy(my_class);
  ClassError:
  unregister_chrdev_region(my_device_nr, 1);
  return -1;
}
static void __exit ModuleExit(void)
{
  gpio_set_value(22, 0);
  gpio_set_value(23, 0);
  gpio_set_value(24, 0);
  gpio_free(22);
  gpio_free(23);
  gpio_free(24);
  cdev_del(&my_device);
  device_destroy(my_class, my_device_nr);
  class_destroy(my_class);
  unregister_chrdev_region(my_device_nr, 1);
}
module_init(ModuleInit);
module_exit(ModuleExit);
