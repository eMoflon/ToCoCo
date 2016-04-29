<?php

namespace App\Analyzer\Helper;

/**
 * Mote Address Trait
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait MoteAddressTrait {
	/**
	 * collects the mote addresses from the log messages
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @return array
	 */
	public function _collectMoteAddresses(array $messages) {
		$addresses = [];
		foreach($messages as $message) {
			$address = null;
			if(preg_match("/^Rime started with address (?P<address_rime>.+)$/", $message->getMessage(), $matches))
				$address = $matches["address_rime"];
			if(preg_match("/^\[network-ipv6\] Routing IPv6 address (?P<address_ipv6>.+)$/", $message->getMessage(), $matches))
				$address = $matches["address_ipv6"];
			
			if($address !== null) {
				if(in_array($address, $addresses))
					trigger_error("mote address $address of logid {$message->getLogid()} is already in use", E_USER_WARNING);
				if(isset($addresses[$message->getMoteid()]))
					trigger_error("mote {$message->getMoteid()} is telling mote address again with logid {$message->getLogid()}", E_USER_WARNING);
				
				$addresses[$message->getMoteid()] = $address;
			}
		}
		
		return $addresses;
	}
}