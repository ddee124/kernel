dd if=/dev/zero of=disk.img bs=1M count=64
parted disk.img -- mklabel gpt
parted disk.img -- mkpart ESP fat32 1MiB 100%
losetup -P /dev/loop0 disk.img
mkfs.vfat -F32 /dev/loop0p1
mkdir /mnt/efi
mount /dev/loop0p1 /mnt/efi
mkdir -p /mnt/efi/EFI/BOOT
grub-install --target=x86_64-efi --efi-directory=/mnt/efi --boot-directory=/mnt/efi/boot --removable
cp system /mnt/efi/boot/
cat <<EOF | sudo tee /mnt/efi/boot/grub/grub.cfg
set timeout=5
set gfxmode=1024x768,auto
set gfxpayload=keep
menuentry "My Multiboot2 Kernel" {
    multiboot2 /boot/system
    boot
}
EOF
umount /mnt/efi
losetup -d /dev/loop0
qemu-img convert -f raw -O vmdk disk.img vmware.vmdk
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive file=disk.img,format=raw -m 4G -vga std -smp 2 -accel kvm -cpu host,+x2apic -machine pc
