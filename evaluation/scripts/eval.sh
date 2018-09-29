EXPERIMENTS=(lmstGen lmstMan kTCMan lkTCMan kTCGen lkTCGen)
pathToEval="/mnt/c/topology-control-evaluation/evaluation"
cd $pathToEval
pathToExperiments=$1
for a in "${EXPERIMENTS[@]}"
do
echo "Go into $pathToExperiments$a"
for d in $pathToExperiments$a/*;
do
	if [ -d "$d" ]; then
	echo "Experiment Path :$d"
	pathToA=$d
	./cli.php --testbed flocklab --evaluation graph-neighborhood --source $pathToA/serial.csv --destination $pathToA/original.png --graph-minute 10
	./cli.php --testbed flocklab --evaluation graph-neighborhood --source $pathToA/serial.csv --destination $pathToA/end.png --graph-minute 15
	./cli.php --testbed flocklab --evaluation energy-milliamperehour --source $pathToA/serial.csv --destination $pathToA/energy.csv
	sed -i -e 's/,/;/g' $pathToA/energy.csv
	sed -i -e 's/\./,/g' $pathToA/energy.csv
	./a.out $pathToA flocklab
	fi
done
size $pathToExperiments$a/$a.sky | python -c '
import sys
for line in sys.stdin:
	r=line.strip("\n").split(None,6)
	fn=r.pop()
	print ";".join(r)+ ";"+fn
'> $pathToExperiments$a/memory.csv
done
