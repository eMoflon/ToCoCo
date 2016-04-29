<?php

namespace App\Testbed;

use App\LogReader\Format\Flocklab as FlocklabLogReader;

/**
 * Flocklab
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class Flocklab implements TestbedInterface {
	/**
	 * get log reader for testbed
	 * 
	 * @return App\LogReader\ReaderInterface
	 */
	public function getLogReader() {
		return new FlocklabLogReader();
	}
	
	/**
	 * get mote positions for testbed
	 *
	 * @return array
	 */
	public function getMotePositions() {
		return [
			1 => ["x" => 22, "y" => 25],
			2 => ["x" => 18, "y" => 105],
			3 => ["x" => 90, "y" => 239],
			4 => ["x" => 69, "y" => 65],
			6 => ["x" => 95, "y" => 344],
			7 => ["x" => 679, "y" => 121],
			8 => ["x" => 80, "y" => 130],
			10 => ["x" => 360, "y" => 200],
			11 => ["x" => 561, "y" => 120],
			13 => ["x" => 613, "y" => 293],
			14 => ["x" => 628, "y" => 173],
			15 => ["x" => 128, "y" => 121],
			16 => ["x" => 85, "y" => 404],
			17 => ["x" => 575, "y" => 333],
			18 => ["x" => 233, "y" => 399],
			19 => ["x" => 505, "y" => 345],
			20 => ["x" => 475, "y" => 324],
			22 => ["x" => 159, "y" => 345],
			23 => ["x" => 410, "y" => 340],
			24 => ["x" => 321, "y" => 393],
			25 => ["x" => 571, "y" => 234],
			26 => ["x" => 464, "y" => 217],
			27 => ["x" => 270, "y" => 397],
			28 => ["x" => 159, "y" => 304],
			31 => ["x" => 221, "y" => 218],
			32 => ["x" => 194, "y" => 191],
			33 => ["x" => 71, "y" => 192],
			200 => ["x" => 242, "y" => 532],
			201 => ["x" => 157, "y" => 775],
			202 => ["x" => 220, "y" => 640],
			204 => ["x" => 144, "y" => 543]
		];
	}
}
