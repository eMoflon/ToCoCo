<?php

namespace App\Analyzer\Helper;

use App\Analyzer\Exception;

/**
 * Ignored Link Trait
 *
 * Parses ignored link information
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait IgnoredLinksTrait {
	/**
	 * collects and preprocesses ignored link information
	 * 
	 * The information will be returned in the format:
	 * array(
	 *     :minute-0 => :information,
	 *     :minute-1 => :information,
	 *     ...,
	 *     :minute-n => :information,
	 * )
	 * and :information has the following structure:
	 * array(
	 *     :mote-x => :ignoredlinks,
	 *     :mote-y => :ignoredlinks,
	 *     :mote-z => :ignoredlinks
	 * )
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @param array $address2moteid
	 * @return array
	 */
	protected function _collectIgnoredLinks(array $messages, array $address2moteid) {
		$starttime = $messages[0]->getTimestamp();
		
		$ignoredlinks = [];
		foreach($messages as $message) {
			if(preg_match("/^\[evaluation\] ignoredlinks: (?P<ignoredlinks>.*)$/i", $message->getMessage(), $matches)) {
				$minute = (int) round(($message->getTimestamp() - $starttime) / 60);
				if(!isset($ignoredlinks[$minute]))
					$ignoredlinks[$minute] = [];
				if(!isset($ignoredlinks[$minute][$message->getMoteid()]))
					$ignoredlinks[$minute][$message->getMoteid()] = [];
				
				if(!$matches["ignoredlinks"])
					continue;
				
				foreach(explode(", ", trim($matches["ignoredlinks"])) as $ignoredlink) {
					if(!isset($address2moteid[$ignoredlink])) {
						//throw new Exception("unknown mote address '" . $ignoredlink . "' of logid " . $message->getLogid());
            printf("[Warning]: Unknown mote address '" . $ignoredlink . "' of logid " . $message->getLogid() . "\n");
            continue;
          }
					
					$ignoredlinks[$minute][$message->getMoteid()][] = $address2moteid[$ignoredlink];
				}
			}
		}
		
		return $ignoredlinks;
	}
}