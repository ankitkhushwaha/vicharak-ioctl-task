

# PreRequiste task for vicharak kernel dev role
What i did

- Utilized kfifo for the circular buffer.
- created a character device `/dev/ankit`
- change data structure of `struct data->data` to static char array, to make it less error prone. 

[demo](demo.mp4)

## Build the kernel Module and Install
```
cd kernel_space/
make
sudo insmod ioctl_kfifo.ko
```

## Build the userspace

```
cd user_space/

gcc -o configurator configurator.c
gcc -o filler filler.c
gcc -o reader reader.c
```

## Run userspace code in Order

```
sudo -s

./configurator
./filler
./reader
```