<?php

namespace App\Testbed;

/**
 * TestbedInterface
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
interface TestbedInterface {
	/**
	 * get log reader for testbed
	 * 
	 * @return App\LogReader\ReaderInterface
	 */
	public function getLogReader();
	
	/**
	 * get mote positions for testbed
	 * 
	 * @return array
	 */
	public function getMotePositions();
}