<?php

namespace App\Analyzer\Helper;

/**
 * Energy Trait
 *
 * Parses the energy information from the mote output and will relate it to the closest
 * minute since the simulation's starting time to pretend motes running perfectly time
 * synchronized. 
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait EnergyTrait {
	/**
	 * collects and preprocesses mote energy information
	 * 
	 * The information will be returned in the format:
	 * array(
	 *     :moteid-1 => :information,
	 *     :moteid-2 => :information,
	 *     ...,
	 *     :moteid-n => :information,
	 * )
	 * and :information has the following structure:
	 * array(
	 *     :minute-1 => :stats,
	 *     :minute-2 => :stats,
	 *     ...,
	 *     :minute-n => :stats,
	 * )
	 * 
	 * @param App\LogReader\Message[] $messages
	 * @param boolean $calculateMilliampereHours
	 * @return array
	 */
	protected function _collectMoteEnergy(array $messages, $calculateMilliampereHours) {
		$energy = [];
		$lastEnergyOfMote = [];
		$powerprofileOfMote = [];
		
		$starttime = $messages[0]->getTimestamp();
		foreach($messages as $message) {
			$pattern = "/energy-power: cpu_active=(?P<cpu_active>\d+(\.\d+)?), cpu_lpm=(?P<cpu_lpm>\d+(\.\d+)?), cpu_irq=(?P<cpu_irq>\d+(\.\d+)?), radio_listen=(?P<radio_listen>\d+(\.\d+)?), radio_transmit=\[(?P<radio_transmit_powerlevels>[^]]*)\]/";
			if(preg_match($pattern, $message->getMessage(), $matches)) {
				$powerprofileOfMote[$message->getMoteid()] = $this->_parsePowerProfile($matches);
			}
			
			$pattern = "/energy-ticks: ticks_per_second=(?P<ticks_per_second>\d+), cpu_active=(?P<cpu_active>\d+), cpu_lpm=(?P<cpu_lpm>\d+), cpu_irq=(?P<cpu_irq>\d+), radio_listen=(?P<radio_listen>\d+), radio_transmit\((?P<maximum_txpower>\d+)\)=(?P<radio_transmit>\d+)\[(?P<radio_transmit_powerlevels>[^]]*)\]/";
			if(preg_match($pattern, $message->getMessage(), $matches)) {
				if(!isset($energy[$message->getMoteid()]))
					$energy[$message->getMoteid()] = [];
				
				// make all values integers (better memory efficiency)
				$radioTransmitPowerLevels = $matches["radio_transmit_powerlevels"];
				$matches = array_map("intval", $matches);
				
				// save all helper information to local variables
				$ticks_per_second = $matches["ticks_per_second"];
				$maximum_txpower = $matches["maximum_txpower"];
				
				// remove all numeric matches (these are "temporary values") and helper information
				foreach($matches as $key => $_value) {
					if(is_numeric($key))
						unset($matches[$key]);
				}
				unset($matches["ticks_per_second"]);
				unset($matches["maximum_txpower"]);
				
				// parse powerlevels from json-like structure
				$powerlevels = [];
				foreach(explode(",", $radioTransmitPowerLevels) as $powerlevelStr) {
					if($powerlevelStr === "")
						continue;
						
					$powerlevel = (int) explode(":", $powerlevelStr)[0];
					$ticks = (int) explode(":", $powerlevelStr)[1];
					$powerlevels[$powerlevel] = $ticks;
				}
				$matches["radio_transmit_powerlevels"] = $powerlevels;
		
				// save empty ticks for every unused radio powerlevel (omitted from log to be more sparse)
				for($powerlevel = 0; $powerlevel <= $maximum_txpower; $powerlevel++) {
					if(!isset($matches["radio_transmit_powerlevels"][$powerlevel])) {
						$matches["radio_transmit_powerlevels"][$powerlevel] = 0;
					}		
				}
				ksort($matches["radio_transmit_powerlevels"]);
				
				// remove ticks of last energy profile from actual energy profile
				$last = @$lastEnergyOfMote[$message->getMoteid()];
				$lastEnergyOfMote[$message->getMoteid()] = $matches;
				foreach($matches as $energysource => $energyvalue) {
					if(!is_array($energyvalue)) {
						$matches[$energysource] = $energyvalue - @$last[$energysource];
					} else {
						foreach($energyvalue as $subenergysource => $subenergyvalue) {
							$matches[$energysource][$subenergysource] = $subenergyvalue - @$last[$energysource][$subenergysource];
						}
					}
				}
				
				// transform ticks to mAh
				if($calculateMilliampereHours) {
					foreach($matches as $energysource => $energyvalue) {
						if(!is_array($energyvalue)) {
							if($energysource != "radio_transmit") {
								$matches[$energysource] = $energyvalue / $ticks_per_second * $powerprofileOfMote[$message->getMoteid()][$energysource];
							}
						} else {
							foreach($energyvalue as $subenergysource => $subenergyvalue) {
								$matches[$energysource][$subenergysource] = $subenergyvalue / $ticks_per_second * $powerprofileOfMote[$message->getMoteid()][$energysource][$subenergysource];
							}
						}
					}
					$matches["radio_transmit"] = array_sum($matches["radio_transmit_powerlevels"]);
					
					$matches["complete_energy"] = 0;
					foreach($matches as $energyvalue) {
						if(is_numeric($energyvalue)) {
							$matches["complete_energy"] += $energyvalue;
						}
					}
				}
				
				$minute = (int) round(($message->getTimestamp() - $starttime) / 60);
				$energy[$message->getMoteid()][$minute] = $matches;
			}
		}
		
		// overall energy profile of a mote
		foreach($energy as $moteid => $minutes) {
			$overall = [];
			foreach(array_keys($minutes[0]) as $energysource) {
				if(!is_array($minutes[0][$energysource])) {
					$overall[$energysource] = array_sum(array_column($minutes, $energysource));
				} else {
					$overall[$energysource] = [];
					foreach(array_keys($minutes[0][$energysource]) as $subenergysource) {
						$overall[$energysource][$subenergysource] = array_sum(array_column(array_column($minutes, $energysource), $subenergysource));
					}
				}
			}
			
			$energy[$moteid][-1] = $overall;
		}
		
		return $energy;
	}
	
	/**
	 * parses the power profile
	 * 
	 * @param array $matches
	 * @return array
	 */
	private function _parsePowerProfile(array $matches) {
		$powerprofile = [];
		foreach($matches as $energysource => $mAh) {
			if(is_numeric($energysource))
				continue;
			
			if(!strpos($mAh, ":")) {
				$powerprofile[$energysource] = (float) $mAh;
			} else {
				$powerprofile[$energysource] = [];
				foreach(explode(",", $mAh) as $subenergy) {
					$powerlevel = (int) explode(":", $subenergy)[0];
					$mAh = (float) explode(":", $subenergy)[1];
					
					$powerprofile[$energysource][$powerlevel] = $mAh;
				}
			}
		}
		
		return $powerprofile;
	}
}