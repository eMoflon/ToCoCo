pathToEval="/mnt/d/topology-control-evaluation/evaluation"
cd $pathToEval


pathToA=$1
./cli.php --testbed flocklab --evaluation graph-neighborhood --source $pathToA/serial.csv --destination $pathToA/original.png --graph-minute 10
./cli.php --testbed flocklab --evaluation graph-neighborhood --source $pathToA/serial.csv --destination $pathToA/end.png --graph-minute 19
./cli.php --testbed flocklab --evaluation energy-milliamperehour --source $pathToA/serial.csv --destination $pathToA/energy.csv
sed -i -e 's/,/;/g' $pathToA/energy.csv
sed -i -e 's/\./,/g' $pathToA/energy.csv
./a.out $pathToA flocklab
