<?php

namespace App\Analyzer\Helper;

/**
 * Statistics Trait
 *
 * Will compute basic evaluation statics on provided data
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait StatisticsTrait {
	/**
	 * all values must be sorted ascending for the percentile statistic
	 *
	 * @var string
	 */
	protected $PERCENTILE_SORT_ASC = "asc";
	
	/**
	 * all values must be sorted ascending for the percentile statistic
	 *
	 * @var string
	 */
	protected $PERCENTILE_SORT_DESC = "desc";
	
	/**
	 * calculates some mathematical statics on the provided data values
	 *
	 * @param array $values
	 * @param string $percentileSortOrder
	 * @return array
	 */
	protected function _calculateStatistics(array $values, $percentileSortOrder) {
		$sorting = ($percentileSortOrder == $this->PERCENTILE_SORT_ASC) ? "sort" : "rsort";
		$sorting($values, SORT_NUMERIC);
		
		return [
			"sum" => array_sum($values),
			"avg" => array_sum($values) / count($values),
			"variance" => $this->_variance($values),
			"stddev" => sqrt($this->_variance($values)),
			"95percentile" => $values[ceil(0.95 * (count($values) - 1))]
		];
	}
	
	/**
	 * calculates the variance of the provided data values
	 * 
	 * @param array $values
	 * @return number
	 */
	private function _variance(array $values) {
		$mean = array_sum($values) / count($values);
		$sumOfSquares = 0;
		foreach($values as $value)
			$sumOfSquares += pow($value - $mean, 2);
	
		return $sumOfSquares / count($values);
	}
}