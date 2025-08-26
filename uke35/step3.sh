cd rootfs/bin
for tool in sh ls cat echo mount; do
    ln -s busybox $tool
done
