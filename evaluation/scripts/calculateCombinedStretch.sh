data="/mnt/c/Users/david/Desktop/evaluationsergebnisse"
eval="/mnt/d/Repositorys/topology-control-evaluation/evaluation"
data=$1
data=$2
cd $eval
find $data -type d |while read dir; do
    echo $dir
    find $dir \( -name "serial*.csv" -a -not -name "*stretch*" \)|while read fname; do
        filename="${fname%.*}"
       ./cli.php --destination $filename"_stretch.csv" --evaluation stretch-plot --source $fname --testbed flocklab
    done
    merge="--merge "
    find $dir -name "*stretch.csv"|(while read fname; do
        merge=$merge$fname","
    done
    ./cli.php --destination $dir/stretch_merge.csv --evaluation stretch-plot --testbed flocklab $merge)
    gnuplot -e "dir='$dir'" stretchPlot.gp
done
