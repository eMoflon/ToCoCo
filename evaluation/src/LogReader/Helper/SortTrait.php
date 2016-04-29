<?php

namespace App\LogReader\Helper;

use App\LogReader\Message;

/**
 * SortTrait
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait SortTrait {
	/**
	 * sorts the log messages in the order they happened
	 * 
	 * @param App\LogReader\Message[] &$messages
	 */
	protected function _sortMessages(array &$messages) {
		usort($messages, function(Message $a, Message $b) {
			if($a->getTimestamp() !== $b->getTimestamp())
				// float values will be casted to integers, so move sub second difference to integer part
				return 1000000 * ($a->getTimestamp() - $b->getTimestamp());
			
			return $a->getLogid() - $b->getLogid();
		});
	}
}