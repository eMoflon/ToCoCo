<?php

namespace App\LogReader\Format;

use App\LogReader\Exception;
use App\LogReader\Message;
use App\LogReader\ReaderInterface;
use App\LogReader\Helper\SortTrait;
use \DateTime;

/**
 * Tudunet LogReader
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class Tudunet implements ReaderInterface {
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
		while($line = fgetcsv($handle, 0, ";")) {
			if(!is_numeric($line[0]))
				continue;
			
			$timestamp = (float) DateTime::createFromFormat("Y-m-d H:i:s.u", $line[1])->format("U.u");
			$logmessage = trim($line[2]);
			$logid++;
			if($keepDebuggingMessages || strpos($logmessage, "DEBUG:") !== 0) {
				$messages[] = new Message($timestamp, intval($line[0]), $logmessage, $logid);
			}
			
			if(strpos($logmessage, "ERROR") === 0) {
				trigger_error("'$logmessage' @ moteid={$line[0]}, logid=$logid", E_USER_WARNING);
			}
		}
		
		$this->_sortMessages($messages);
		return $messages;
	}
}