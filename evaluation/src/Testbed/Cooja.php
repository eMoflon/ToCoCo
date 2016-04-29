<?php

namespace App\Testbed;

use App\LogReader\Format\Cooja as CoojaLogReader;

/**
 * Cooja
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class Cooja implements TestbedInterface {
	/**
	 * get log reader for testbed
	 * 
	 * @return App\LogReader\ReaderInterface
	 */
	public function getLogReader() {
		return new CoojaLogReader();
	}
	
	/**
	 * get mote positions for testbed
	 *
	 * @return array
	 */
	public function getMotePositions() {
		// cooja has no predefined positions
		return [];
	}
}