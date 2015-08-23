var RECORD_BIT1_EVENT		= 0x80
var RECORD_BIT1_FLAGS		= 0x40
var RECORD_BIT1_NAV			= 0x20
var RECORD_BIT1_ALT			= 0x10
var RECORD_BIT1_COG			= 0x08
var RECORD_BIT1_RS485_1		= 0x04
var RECORD_BIT1_RS485_2		= 0x02
var RECORD_BIT_MORE			= 0x01

var RECORD_BIT2_ADC1		= 0x80
var RECORD_BIT2_ADC2		= 0x40
var RECORD_BIT2_ADC3		= 0x20
var RECORD_BIT2_FREQUENCY1	= 0x10
var RECORD_BIT2_FREQUENCY2	= 0x08
var RECORD_BIT2_FREQUENCY3	= 0x04
var RECORD_BIT2_FREQUENCY4	= 0x02
var RECORD_BIT2_MORE_1		= 0x01

var RECORD_BIT3_VCC			= 0x80
var RECORD_BIT3_SAT_NO		= 0x40
var RECORD_BIT3_ADC4		= 0x20
var RECORD_BIT3_COUNTER1	= 0x10
var RECORD_BIT3_COUNTER2	= 0x08
var RECORD_BIT3_COUNTER3	= 0x04
var RECORD_BIT3_COUNTER4	= 0x02
var RECORD_BIT3_MORE_2		= 0x01

var RECORD_BIT4_RS232_1		= 0x80
var RECORD_BIT4_RS232_2		= 0x40
var RECORD_BIT4_ODOMETER	= 0x20
var RECORD_BIT4_FREQUENCY5	= 0x10
var RECORD_BIT4_FREQUENCY6	= 0x08
var RECORD_BIT4_FREQUENCY7	= 0x04
var RECORD_BIT4_FREQUENCY8	= 0x02
var RECORD_BIT4_MORE_3		= 0x01

var RECORD_BIT5_ADC5		= 0x80
var RECORD_BIT5_ADC6		= 0x40
var RECORD_BIT5_COUNTER5	= 0x20
var RECORD_BIT5_COUNTER6	= 0x10
var RECORD_BIT5_COUNTER7	= 0x08
var RECORD_BIT5_COUNTER8	= 0x04
var RECORD_BIT5_INJECTOR	= 0x02
var RECORD_BIT5_MORE_4		= 0x01

var RECORD_FLAG1_IGNITION	= 0x80
var RECORD_FLAG1_ENGINE		= 0x40
var RECORD_FLAG1_MOVE		= 0x20
var RECORD_FLAG1_COG_9		= 0x10
var RECORD_FLAG1_SPEED_9	= 0x08
var RECORD_FLAG1_SPEED_10	= 0x04
var RECORD_FLAG1_SPEED_11	= 0x02
var RECORD_FLAG1_MORE		= 0x01

var RECORD_FLAG2_DI1		= 0x80
var RECORD_FLAG2_DI2		= 0x40
var RECORD_FLAG2_DI3		= 0x20
var RECORD_FLAG2_DI4		= 0x10
var RECORD_FLAG2_DI5		= 0x08
var RECORD_FLAG2_WAIT		= 0x04
var RECORD_FLAG2_SLEEP		= 0x02
var RECORD_FLAG2_MORE		= 0x01

var RECORD_EVENT_TERMINAL_ONLINE		= 0x0001
var RECORD_EVENT_TERMINAL_OFFLINE 		= 0x0002
var RECORD_EVENT_REASON					= 0x0003
var RECORD_EVENT_TERMINAL_UPDATE_START	= 0x0020
var RECORD_EVENT_TERMINAL_UPDATE_FAILED	= 0x0022
var RECORD_EVENT_TERMINAL_UPDATE_DONE	= 0x0023
var RECORD_EVENT_TERMINAL_PROFILE		= 0x0024

function get64u(data, i) {

	return data[i] + (data[i + 4] << 8) + (data[i + 8] << 16) + (data[i + 12] << 24) + (data[i + 16] << 32) + (data[i + 20] << 40) + (data[i + 24] << 48) + (data[i + 28] << 56);
}

function get32u(data, i) {

	return data[i] + (data[i + 4] << 8) + (data[i + 8] << 16) + (data[i + 12] << 24);
}

function get16u(data, i) {

	return data[i] + (data[i + 4] << 8);
}

function GetPngHistory(record, from, to, onSuccess, onFailure) {
	
	var image = new Image();
	
	image.onload = function() {
        
		var width = image.width;
		var height = image.height;

		if ((width == 1)&&(height == 1)) {
			if (onSuccess)
				onSuccess();
			return;
		}

		var canvas = document.createElement('canvas');
		canvas.width = width;
		canvas.height = height;

		var context = canvas.getContext("2d");
		context.drawImage(image, 0, 0);

		var imageData = context.getImageData(0, 0, width, height);
		var data = imageData.data;
		var len = data.length;

		var pointsCount = 0;
		var i;

		for (i = 0; i < len; pointsCount++) {

			if (i == len - 4) {
				i = len;
				break;
			}

			var size = data[i] + (data[i + 4] << 8);

			i += size * 4;

			if (size == 0xCDCD) {
				i = len;
				break;
			}
		}

		if (i != len) {
			console.log('wrong data');
			if (onFailure)
				onFailure();

			return;
		}

		var result = {

			time:	new Uint32Array(pointsCount),
			bits1:	new Uint8Array(pointsCount),
			bits2:	new Uint8Array(pointsCount),
			bits3:	new Uint8Array(pointsCount),
			bits4:	new Uint8Array(pointsCount),
			bits5:	new Uint8Array(pointsCount),
		
			event:	new Uint8Array(pointsCount),

			flags1: new Uint8Array(pointsCount),
			flags2: new Uint8Array(pointsCount),

			lat:	new Float32Array(pointsCount),
			lng:	new Float32Array(pointsCount),
			speed:	new Uint16Array(pointsCount),

			bit1_mask: RECORD_BIT1_EVENT | RECORD_BIT1_FLAGS | RECORD_BIT1_NAV | RECORD_BIT_MORE,
			bit2_mask: RECORD_BIT_MORE,
			bit3_mask: RECORD_BIT_MORE,
			bit4_mask: RECORD_BIT_MORE,
			bit5_mask: RECORD_BIT_MORE
		};

		var caps = record.raw.caps;

		if (caps.analog1) {
			result.analog1 = new Uint16Array(pointsCount); 
			result.bit2_mask |= RECORD_BIT2_ADC1;
		}
		if (caps.analog2) {
			result.analog2 = new Uint16Array(pointsCount); 
			result.bit2_mask |= RECORD_BIT2_ADC2;
		}
		if (caps.analog3) {
			result.analog3 = new Uint16Array(pointsCount); 
			result.bit2_mask |= RECORD_BIT2_ADC3;
		}
		if (caps.analog4) {
			result.analog4 = new Uint16Array(pointsCount); 
			result.bit3_mask |= RECORD_BIT3_ADC4;
		}
		if (caps.analog5) {
			result.analog5 = new Uint16Array(pointsCount); 
			result.bit5_mask |= RECORD_BIT5_ADC5;
		}
		if (caps.analog6) {
			result.analog6 = new Uint16Array(pointsCount); 
			result.bit5_mask |= RECORD_BIT5_ADC6;
		}
		if (caps.analog7) {
			result.analog7 = new Uint16Array(pointsCount); 
		}
		if (caps.analog8) {
			result.analog8 = new Uint16Array(pointsCount); 
		}

		if (caps.frequency1) {
			result.frequency1 = new Uint16Array(pointsCount); 
			result.bit2_mask |= RECORD_BIT2_FREQUENCY1;
		}
		if (caps.frequency2) {
			result.frequency2 = new Uint16Array(pointsCount); 
			result.bit2_mask |= RECORD_BIT2_FREQUENCY2;
		}
		if (caps.frequency3) {
			result.frequency3 = new Uint16Array(pointsCount); 
			result.bit2_mask |= RECORD_BIT2_FREQUENCY3;
		}
		if (caps.frequency4) {
			result.frequency4 = new Uint16Array(pointsCount); 
			result.bit2_mask |= RECORD_BIT2_FREQUENCY4;
		}
		if (caps.frequency5) {
			result.frequency5 = new Uint16Array(pointsCount); 
			result.bit4_mask |= RECORD_BIT4_FREQUENCY5;
		}
		if (caps.frequency6) {
			result.frequency6 = new Uint16Array(pointsCount); 
			result.bit4_mask |= RECORD_BIT4_FREQUENCY6;
		}
		if (caps.frequency7) {
			result.frequency7 = new Uint16Array(pointsCount); 
			result.bit4_mask |= RECORD_BIT4_FREQUENCY7;
		}
		if (caps.frequency8) {
			result.frequency8 = new Uint16Array(pointsCount); 
			result.bit4_mask |= RECORD_BIT4_FREQUENCY8;
		}

		if (caps.counter1) {
			result.counter1 = new Uint16Array(pointsCount); 
			result.bit3_mask |= RECORD_BIT3_COUNTER1;
		}
		if (caps.counter2) {
			result.counter2 = new Uint16Array(pointsCount); 
			result.bit3_mask |= RECORD_BIT3_COUNTER2;
		}
		if (caps.counter3) {
			result.counter3 = new Uint16Array(pointsCount); 
			result.bit3_mask |= RECORD_BIT3_COUNTER3;
		}
		if (caps.counter4) {
			result.counter4 = new Uint16Array(pointsCount); 
			result.bit3_mask |= RECORD_BIT3_COUNTER4;
		}
		if (caps.counter5) {
			result.counter5 = new Uint16Array(pointsCount); 
			result.bit5_mask |= RECORD_BIT3_COUNTER5;
		}
		if (caps.counter6) {
			result.counter6 = new Uint16Array(pointsCount); 
			result.bit5_mask |= RECORD_BIT3_COUNTER6;
		}
		if (caps.counter7) {
			result.counter7 = new Uint16Array(pointsCount); 
			result.bit5_mask |= RECORD_BIT3_COUNTER7;
		}
		if (caps.counter8) {
			result.counter8 = new Uint16Array(pointsCount); 
			result.bit5_mask |= RECORD_BIT3_COUNTER8;
		}

		if (caps.cog) {
			result.cog = new Uint8Array(pointsCount);
			result.bit1_mask |= RECORD_BIT1_COG;
		}

		if (caps.sat_count) {
			result.sat_count = new Uint8Array(pointsCount);
			result.bit3_mask |= RECORD_BIT3_SAT_NO;
		}

		if (caps.altitude) {
			result.altitude = new Uint16Array(pointsCount);
			result.bit1_mask |= RECORD_BIT1_ALT;
		}

		if (caps.rpm) {
			result.rpm = new Uint16Array(pointsCount);
		}

		if (caps.rs485) {
			result.rs485_1 = new Uint16Array(pointsCount);
			result.rs485_2 = new Uint16Array(pointsCount);
			result.bit1_mask |= RECORD_BIT1_RS485_1;
			result.bit1_mask |= RECORD_BIT1_RS485_2;
		}

		if (caps.rs232_1) {
			result.rs232_1 = new Uint16Array(pointsCount);
			result.bit4_mask |= RECORD_BIT4_RS232_1;
		}

		if (caps.rs232_2) {
			result.rs232_1 = new Uint16Array(pointsCount);
			result.bit4_mask |= RECORD_BIT4_RS232_2;
		}

		if (caps.vcc) {
			result.vcc = new Uint16Array(pointsCount);
			result.bit3_mask |= RECORD_BIT3_VCC;
		}

		if (caps.injector) {
			result.injector = new Uint32Array(pointsCount);
			result.bit5_mask |= RECORD_BIT5_INJECTOR;
		}

		if (caps.odometr) {
			result.odometr = new Uint32Array(pointsCount);
			result.bit4_mask |= RECORD_BIT4_ODOMETER;
		}

		var arr_time		= result.time;
		
		var arr_bits1		= result.bits1;
		var arr_bits2		= result.bits2;
		var arr_bits3		= result.bits3;
		var arr_bits4		= result.bits4;
		var arr_bits5		= result.bits5;

		var arr_event		= result.event;
		var arr_flags1		= result.flags1;
		var arr_flags2		= result.flags2;

		var arr_lat			= result.lat;
		var arr_lng			= result.lng;
		var arr_speed		= result.speed;

		var arr_analog1		= result.analog1;
		var arr_analog2		= result.analog2;
		var arr_analog3		= result.analog3;
		var arr_analog4		= result.analog4;
		var arr_analog5		= result.analog5;
		var arr_analog6		= result.analog6;
		var arr_analog7		= result.analog7;
		var arr_analog8		= result.analog8;

		var arr_frequency1	= result.frequency1;
		var arr_frequency2	= result.frequency2;
		var arr_frequency3	= result.frequency3;
		var arr_frequency4	= result.frequency4;
		var arr_frequency5	= result.frequency5;
		var arr_frequency6	= result.frequency6;
		var arr_frequency7	= result.frequency7;
		var arr_frequency8	= result.frequency8;

		var arr_counter1	= result.counter1;
		var arr_counter2	= result.counter2;
		var arr_counter3	= result.counter3;
		var arr_counter4	= result.counter4;
		var arr_counter5	= result.counter5;
		var arr_counter6	= result.counter6;
		var arr_counter7	= result.counter7;
		var arr_counter8	= result.counter8;

		var arr_cog			= result.cog;
		var arr_sat_count	= result.sat_count;
		var arr_altitude	= result.altitude;
		var arr_rpm			= result.rpm;
		var arr_rs485_1		= result.rs485_1;
		var arr_rs485_2		= result.rs485_2;
		var arr_rs232_1		= result.rs232_1;
		var arr_rs232_2		= result.rs232_2;
		var arr_vcc			= result.vcc;
		var arr_injector	= result.injector;
		var arr_odometr		= result.odometr;

		var bit1_mask		= result.bit1_mask;
		var bit2_mask		= result.bit2_mask;
		var bit3_mask		= result.bit3_mask;
		var bit4_mask		= result.bit4_mask;
		var bit5_mask		= result.bit5_mask;

		var iPoint = 0;

		for (i = 0; i < len - 4; iPoint++) {

			var size = data[i] + (data[i + 4] << 8);

			if (size == 0xCDCD) {
				break;
			}

			var next_record = i + size * 4;

			i += 8;

			arr_time[iPoint]  = get32u(data, i); i+= 16;

			var bits1 = 0;
			var bits2 = 0;
			var bits3 = 0;
			var bits4 = 0;
			var bits5 = 0;

			bits1 = data[i]; i+= 4;

			if (bits1 & RECORD_BIT_MORE) {

				bits2 = data[i]; i+= 4;

				if (bits2 & RECORD_BIT_MORE) {

					bits3 = data[i]; i+= 4;

					if (bits3 & RECORD_BIT_MORE) {

						bits4 = data[i]; i+= 4;

						if (bits4 & RECORD_BIT_MORE) {

							bits5 = data[i];

							while (data[i] & RECORD_BIT_MORE)
								i += 4;

							i += 4;
						}
					}
				}
			}

			arr_bits1[iPoint] = bits1;
			arr_bits2[iPoint] = bits2;
			arr_bits3[iPoint] = bits3;
			arr_bits4[iPoint] = bits4;
			arr_bits5[iPoint] = bits5;

			if (bits1 & RECORD_BIT1_EVENT) {
				arr_event[iPoint] = get16u(data, i); i+= 8;
			}

			var flags1 = 0;

			if (bits1 & RECORD_BIT1_FLAGS) {

				flags1 = data[i]; i+= 4;
				arr_flags1[iPoint] = flags1;
			
				if (arr_flags1[iPoint] & RECORD_FLAG1_MORE) {
				
					arr_flags2[iPoint] = data[i];

					while (data[i] & RECORD_FLAG1_MORE)
						i += 4;

					i += 4;
				}
			}

			if (bits1 & RECORD_BIT1_NAV) {

				arr_lat[iPoint] = get32u(data, i) / 10000000; i += 16;
				arr_lng[iPoint] = get32u(data, i) / 10000000; i += 16;

				var speed = data[i]; i+= 4;

				if (flags1 & 0x08)
					speed |= 0x100;

				if (flags1 & 0x04)
					speed |= 0x200;

				if (flags1 & 0x02)
					speed |= 0x400;

				arr_speed[iPoint] = speed;

				if (speed == 0x07FF) {
					arr_bits1[iPoint] &= ~RECORD_BIT1_NAV;
				}
			}

			if (bits1 & RECORD_BIT1_ALT) {
				if (bit1_mask & RECORD_BIT1_ALT)
					arr_altitude[iPoint] = get16u(data, i); 
				i+= 8;
			}

			if (bits1 & RECORD_BIT1_COG) {				
				if (bit1_mask & RECORD_BIT1_COG)
					arr_cog[iPoint] = data[i];
				i+= 4;
			}

			if (bits1 & RECORD_BIT1_RS485_1) {
				if (bit1_mask & RECORD_BIT1_RS485_1)
					arr_rs485_1[iPoint] = get16u(data, i); 
				i+= 8;
			}

			if (bits1 & RECORD_BIT1_RS485_2) {
				if (bit1_mask & RECORD_BIT1_RS485_2)
					arr_rs485_2[iPoint] = get16u(data, i); 
				i+= 8;
			}

			if (bits2 !== 0) {

				if (bits2 & RECORD_BIT2_ADC1) {
					if (bit2_mask & RECORD_BIT2_ADC1)
						arr_analog1[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_ADC2) {
					if (bit2_mask & RECORD_BIT2_ADC2)
						arr_analog2[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_ADC3) {
					if (bit2_mask & RECORD_BIT2_ADC3)
						arr_analog3[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY1) {
					if (bit2_mask & RECORD_BIT2_FREQUENCY1)
						arr_frequency1[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY2) {
					if (bit2_mask & RECORD_BIT2_FREQUENCY2)
						arr_frequency2[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY3) {
					if (bit2_mask & RECORD_BIT2_FREQUENCY3)
						arr_frequency3[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY4) {
					if (bit2_mask & RECORD_BIT2_FREQUENCY4)
						arr_frequency4[iPoint] = get16u(data, i); 
					i+= 8;
				}

				if (bits3 !== 0) {

					if (bits3 & RECORD_BIT3_VCC) {
						if (bit3_mask & RECORD_BIT3_VCC)
							arr_vcc[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_SAT_NO) {
						if (bit3_mask & RECORD_BIT3_SAT_NO)
							arr_sat_count[iPoint] = data[i]; 
						i+= 4;
					}
					if (bits3 & RECORD_BIT3_ADC4) {
						if (bit3_mask & RECORD_BIT3_ADC4)
							arr_analog4[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER1) {
						if (bit3_mask & RECORD_BIT3_COUNTER1)
							arr_counter1[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER2) {
						if (bit3_mask & RECORD_BIT3_COUNTER2)
							arr_counter2[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER3) {
						if (bit3_mask & RECORD_BIT3_COUNTER3)
							arr_counter3[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER4) {
						if (bit3_mask & RECORD_BIT3_COUNTER4)
							arr_counter4[iPoint] = get16u(data, i); 
						i+= 8;
					}

					if (bits4 !== 0) {

						if (bits4 & RECORD_BIT4_RS232_1) {
							if (bit4_mask & RECORD_BIT4_RS232_1)
								arr_rs232_1[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_RS232_2) {
							if (bit4_mask & RECORD_BIT4_RS232_2)
								arr_rs232_2[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_ODOMETER) {
							if (bit4_mask & RECORD_BIT4_ODOMETER)
								arr_odometr[iPoint] = get32u(data, i); 
							i+= 16;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY5) {
							if (bit4_mask & RECORD_BIT4_FREQUENCY5)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY6) {
							if (bit4_mask & RECORD_BIT4_FREQUENCY6)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY7) {
							if (bit4_mask & RECORD_BIT4_FREQUENCY7)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY8) {
							if (bit4_mask & RECORD_BIT4_FREQUENCY8)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}

						if (bits5 !== 0) {
	
							if (bits5 & RECORD_BIT5_ADC5) {
								if (bit5_mask & RECORD_BIT5_ADC5)
									arr_analog5[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_ADC6) {
								if (bit5_mask & RECORD_BIT5_ADC6)
									arr_analog6[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER5) {
								if (bit5_mask & RECORD_BIT5_COUNTER5)
									arr_counter5[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER6) {
								if (bit5_mask & RECORD_BIT5_COUNTER6)
									arr_counter6[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER7) {
								if (bit5_mask & RECORD_BIT5_COUNTER7)
									arr_counter7[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER8) {
								if (bit5_mask & RECORD_BIT5_COUNTER8)
									arr_counter8[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_INJECTOR) {
								if (bit5_mask & RECORD_BIT5_INJECTOR)
									arr_injector[iPoint] = get32u(data, i); 
								i+= 16;
							}
						}
					}
				}
			}
			
			i = next_record;
		}

		if (onSuccess)
			onSuccess(result);
	};

	image.onerror = onFailure;

	image.src = '/history/' + (record.getId()) + '/' + from + '/' + to + '.png?tc=' + (new Date).getTime();
}
