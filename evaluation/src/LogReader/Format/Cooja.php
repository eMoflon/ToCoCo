<?php

namespace App\LogReader\Format;

use App\LogReader\Exception;
use App\LogReader\Message;
use App\LogReader\ReaderInterface;
use App\LogReader\Helper\SortTrait;

/**
 * Cooja LogReader
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class Cooja implements ReaderInterface {
	use SortTrait;
	
	/**
	 * parses the provided mote output
	 * 
	 * @param string $logFile
	 * @param boolean $keepDebuggingMessages
	 * @return App\LogReader\Message[]
	 */
	public function parseMessages($logFile, $keepDebuggingMessages = true) {
		$handle = fopen($logFile, "r");
		if(!$handle)
			throw new Exception("failed opening '" . $logFile . "'");
		
		$logid = 0;
		$messages = array();
		while($line = fgets($handle)) {
			if(preg_match("/^(?P<time>(\d+(:|\.)?)+)\s+ID:(?P<moteid>\d+)\s+(?P<message>.+)$/", trim($line), $matches)) {
				$timestamp = 0;
				foreach(array_reverse(explode(":", $matches["time"])) as $i => $time)
					$timestamp += pow(60, $i) * $time;
				
				$logid++;
				if($keepDebuggingMessages || strpos($matches["message"], "DEBUG:") !== 0) {
					$messages[] = new Message($timestamp, (int) $matches["moteid"], $matches["message"], $logid);
				}
				
				if(strpos($matches["message"], "ERROR") === 0) {
					trigger_error("'{$matches["message"]}' @ moteid={$matches["moteid"]}, logid=$logid", E_USER_WARNING);
				}
			}
		}
		
		$this->_sortMessages($messages);
		return $messages;
	}
}