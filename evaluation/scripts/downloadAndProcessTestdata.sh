id=$1
target=$2
echo "$target$id"
curl --user d.giessing:ortxfmy1 https://www.flocklab.ethz.ch/user/webdav/$id/results.tar.gz --output $target$id.tar.gz
tar -xzf $target$id.tar.gz -C $target
cd /mnt/d/Repositorys/topology-control-evaluation/evaluation/scripts
./../cli.php --destination $target$id/after.png --evaluation graph-neighborhood --graph-minute 14 --source $target$id/serial.csv --testbed flocklab
./../cli.php --destination $target$id/before.png --evaluation graph-neighborhood --graph-minute 10 --source $target$id/serial.csv --testbed flocklab
./filterLog $target$id flocklab $target$id/filtered.csv