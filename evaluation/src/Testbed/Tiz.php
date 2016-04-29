<?php

namespace App\Testbed;

use App\LogReader\Format\Tudunet as TudunetLogReader;

/**
 * Tiz Tudunet
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class Tiz implements TestbedInterface {
	/**
	 * get log reader for testbed
	 * 
	 * @return App\LogReader\ReaderInterface
	 */
	public function getLogReader() {
		return new TudunetLogReader();
	}
	
	/**
	 * get mote positions for testbed
	 *
	 * @return array
	 */
	public function getMotePositions() {
		$pixelOffset = 75;
		
		$motePositions = [];
		for($moteid = 1001; $moteid <= 1060; $moteid++) {
			$motePositions[$moteid] = [
				"x" => floor(($moteid - 1001) / 5) * $pixelOffset,
				"y" => (($moteid - 1) % 5) * $pixelOffset
			];
		}
		
		return $motePositions;
	}
}