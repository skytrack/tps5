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

function Terminal(jsonObject) 
{
	var i;

	this.id						= 0;
	this.title					= '';
	this.color					= 0;

	this.ignition_field			= '';
	this.ignition_bit			= 0;
	this.ignition_mask_field	= '';
	this.ignition_mask_bit		= 0;
	this.real_ignition			= false;
	
	this.engine_field			= '';
	this.engine_bit				= 0;
	this.engine_mask_field		= '';
	this.engine_mask_bit		= 0;
	this.real_engine			= false;
	
	this.move_field				= '';
	this.move_bit				= 0;
	this.move_mask_field		= '';
	this.move_mask_bit			= 0;
	this.real_move				= false;

	this.lls_left_data_field	= '';
	this.lls_left_mask_field	= '';
	this.lls_left_mask_bit		= 0;
	this.lls_left_factor		= 0;
	this.lls_left_table			= [];

	this.lls_right_data_field	= '';
	this.lls_right_mask_field	= '';
	this.lls_right_mask_bit		= 0;
	this.lls_right_factor		= 0;
	this.lls_right_table		= [];

	if (typeof jsonObject == 'object') 
	{
		this.id				= jsonObject.id;
		this.title			= jsonObject.name;
		this.color			= jsonObject.custom ? jsonObject.custom.color : 'FF0000';
		this.info 			= jsonObject.info;
		this.trueValue		= true;

		var caps = jsonObject.caps;
		
		if (typeof caps == 'object') 
		{
			for (i = 1; i <= 8; i++) 
			{
				if (caps['analog' + i] === true)
					this['capAnalog' + i] = true;
				
				if (caps['discrete' + i] === true)
					this['capDiscrete' + i] = true;
				
				if (caps['frequency' + i] === true)
					this['capFrequency' + i] = true;
				
				if (caps['counter' + i] === true)
					this['capCounter' + i] = true;
				
				if (caps['output' + i] === true)
					this['capOutput' + i] = true;
			}

			if (caps.cog === true)
				this.capCog	= true;
			if (caps.fix3d === true)
				this.capFix3D = true;
			if (caps.sat_count === true)
				this.capSatCount = true;
			if (caps.altitude === true)
				this.capAltitude = true;
			if (caps.ignition === true)
				this.capIgnition = true;
			if (caps.engine === true)
				this.capEngine = true;
			if (caps.move === true)
				this.capMove = true;
			if (caps.alarm === true)
				this.capAlarm = true;
			if (caps.rs485 === true)
				this.capRs485 = true;
			if (caps.rs232_1 === true)
				this.capRs232_1	= true;
			if (caps.rs232_2 === true)
				this.capRs232_2	= true;
			if (caps.voice === true)
				this.capVoice = true;
			if (caps.vcc === true)
				this.capVcc	= true;
			if (caps.injector === true)
				this.capInjector = true;
			if (caps.odometer === true)
				this.capOdometr	= true;
			if (caps.hwsettings === true)
				this.capHwSettings= true;

			this.has_taho = false;

			var sensors	= jsonObject.sensors;

			if (typeof sensors == 'object') 
			{
				for (i = 1; i <= 8; i++)
				{
					var sensor;

					sensor = sensors['analog' + i + '_type'];
					if ((typeof sensor == 'number')&&(sensor >= 0)&&(sensor <= 3)&&(caps['analog' + i] === true)) 
						this['senAnalog' + i] = sensor;

					sensor = sensors['discrete' + i + '_type'];
					if ((typeof sensor == 'number')&&(sensor >= 0)&&(sensor <= 4)&&(caps['discrete' + i] === true))
						this['senDiscrete' + i] = sensor;

					sensor = sensors['frequency' + i + '_type'];
					if ((typeof sensor == 'number')&&(sensor >= 0)&&(sensor <= 3)&&(caps['frequency' + i] === true))
						this['senFrequency' + i] = sensor;

					sensor = sensors['counter' + i + '_type'];
					if ((typeof sensor == 'number')&&(sensor >= 0)&&(sensor <= 2)&&(caps['counter' + i] === true))
						this['senCounter' + i] = sensor;					
				}

				var freq_bit = [0x10, 0x08, 0x04, 0x02, 0x10, 0x08, 0x04, 0x02];
			
				for (i = 1; i <= 8; i++) 
				{
					if (this['senFrequency' + i] == 1) 
					{
						this.taho_field			= 'arr_frequency' + i;
						this.taho_mask_field	= (i <= 4) ? 'arr_bits2' : 'arr_bits4';
						this.taho_mask_bit		= freq_bit[i - 1];
						this.taho_factor		= sensors['frequency' + i + '_factor'] / 60;
						this.has_taho			= true;
						break;
					}
				}
			}

			var bit	= [0x80, 0x40, 0x20, 0x10, 0x08];
				
			this.has_ignition = false;

			if (caps.ignition) 
			{
				this.ignition_field			= 'arr_flags1';
				this.ignition_bit			= RECORD_FLAG1_IGNITION;
				this.ignition_mask_field	= 'arr_bits1'; 
				this.ignition_mask_bit		= RECORD_BIT1_FLAGS;
				this.real_ignition			= true;
				this.has_ignition			= true;
			}
			else 
			{
				for (i = 1; i <= 5; i++) 
				{
					if (this['senDiscrete' + i] == 1) 
					{
						this.ignition_field			= 'arr_flags2';
						this.ignition_bit			= bit[i - 1];
						this.ignition_mask_field	= 'arr_bits1'; 
						this.ignition_mask_bit		= RECORD_BIT1_FLAGS;
						this.real_ignition			= true;
						this.has_ignition			= true;
						break;
					}
				}
			}	

			this.has_engine = false;

			if (caps.engine) 
			{
				this.engine_field		= 'arr_flags1';
				this.engine_bit			= RECORD_FLAG1_ENGINE;
				this.engine_mask_field	= 'arr_bits1'; 
				this.engine_mask_bit	= RECORD_BIT1_FLAGS;
				this.real_engine		= true;
				this.has_engine			= true;
			}
			else 
			{
				for (i = 1; i <= 5; i++) 
				{
					if (this['senDiscrete' + i] == 2) 
				    {
						this.engine_field		= 'arr_flags2';
						this.engine_bit			= bit[i - 1];
						this.engine_mask_field	= 'arr_bits1'; 
						this.engine_mask_bit	= RECORD_BIT1_FLAGS;
						this.real_engine		= true;
						this.has_engine			= true;
						break;
					}
				}
							
				if (i == 6) 
				{
					this.engine_field		= this.ignition_field;
					this.engine_bit			= this.ignition_bit;
					this.engine_mask_field	= this.ignition_mask_field; 
					this.engine_mask_bit	= this.ignition_mask_bit;
					this.real_engine		= false;
				}
			}
			
			this.has_move = false;

			if (caps.move) 
			{
				this.move_field			= 'arr_flags1';
				this.move_bit			= RECORD_FLAG1_MOVE;
				this.move_mask_field	= 'arr_bits1'; 
				this.move_mask_bit		= RECORD_BIT1_FLAGS;
				this.real_move			= true;
				this.has_move			= true;
			}
			else 
			{
				for (i = 1; i <= 5; i++) 
				{
					if (this['senDiscrete' + i] == 3) 
				    {
						this.move_field			= 'arr_flags2';
						this.move_bit			= bit[i - 1];
						this.move_mask_field	= 'arr_bits1'; 
						this.move_mask_bit		= RECORD_BIT1_FLAGS;
						this.real_move			= true;
						this.has_move			= true;
						break;
					}
				}
							
				if (i == 6) {
					this.move_field			= this.engine_field;
					this.move_bit			= this.engine_bit;
					this.move_mask_field	= this.engine_mask_field; 
					this.move_mask_bit		= this.engine_mask_bit;
					this.real_move			= false;
				}
			}
				
			var freq_bit	= [0x10, 0x08, 0x04, 0x02, 0x10, 0x08, 0x04, 0x02];
			var analog_bit	= [0x80, 0x40, 0x20, 0x20, 0x80, 0x40];
					
			var table;
			var table_len;
			var sensor;
			var fuel;
			var sensor_value;
			var fuel_value;

			this.has_lls_left	= false;
			this.has_lls_right	= false;
			this.has_lls		= false;

			if (typeof jsonObject.llsleft == 'object') 
			{
				table = jsonObject.llsleft.table;

				if (typeof table == 'object') 
				{
					table_len = table.length;

					if (table_len > 1) 
					{
						for (i = 1; i <= 8; i++) 
						{
							if (this['senFrequency' + i] == 2) 
							{
								this.lls_left_data_field	= 'arr_frequency' + i;
								this.lls_left_mask_field	= (i <= 4) ? 'arr_bits2' : 'arr_bits4';
								this.lls_left_mask_bit		= freq_bit[i - 1];
								this.lls_left_factor		= 0.1;
	
								break;
							}
						}
	
						if (i == 9) {									
	
							for (i = 1; i <= 6; i++) 
							{
								if (this['senAnalog' + i] == 2)
								{
									this.lls_left_data_field	= 'arr_analog' + i;
									this.lls_left_mask_field	= (i < 4) ? 'arr_bits2' : ((i == 4) ? 'arr_bits3' : 'arr_bits5');
									this.lls_left_mask_bit		= analog_bit[i - 1];
									this.lls_left_factor		= 0.001;
	
									break;
								}
							}
						}

						if (i == 7) {
							if ((this.capRs485)&&(typeof sensors == 'object')&&(sensors.rs485_lls_left)) {
								this.lls_left_data_field	= 'arr_rs485_1';
								this.lls_left_mask_field	= 'arr_bits1';
								this.lls_left_mask_bit		= RECORD_BIT1_RS485_1;
								this.lls_left_factor		= 1;
								i = 0;
							}
						}

						if (i <= 6) {

							sensor	= table[0].sensor_value;
							fuel	= table[0].fuel_value;
	
							for (i = 1; i < table_len; i++) 
							{	
								sensor_value	= table[i].sensor_value;
								fuel_value		= table[i].fuel_value;
	
								table[i - 1].d = (fuel_value - fuel) / (sensor_value - sensor);
	
								sensor	= sensor_value;
								fuel	= fuel_value;
							}

							this.lls_left_table		= table;
							this.has_lls_left		= true;
							this.has_lls			= true;								
						}	
					}
				}
			}
					
			if (typeof jsonObject.llsright == 'object') 
			{
				table = jsonObject.llsright.table;

				if (typeof table == 'object') 
				{
					table_len = table.length;

					if (table_len > 1) 
					{
						for (i = 1; i <= 8; i++) 
						{
							if (this['senFrequency' + i] == 3) 
							{
								this.lls_right_data_field	= 'arr_frequency' + i;
								this.lls_right_mask_field	= (i <= 4) ? 'arr_bits2' : 'arr_bits4';
								this.lls_right_mask_bit		= freq_bit[i - 1];
								this.lls_right_factor		= 0.1;
	
								break;
							}
						}
	
						if (i == 9) {									
	
							for (i = 1; i <= 6; i++) 
							{
								if (this['senAnalog' + i] == 3)
								{
									this.lls_right_data_field	= 'arr_analog' + i;
									this.lls_right_mask_field	= (i < 4) ? 'arr_bits2' : ((i == 4) ? 'arr_bits3' : 'arr_bits5');
									this.lls_right_mask_bit		= analog_bit[i - 1];
									this.lls_right_factor		= 0.001;
	
									break;
								}
							}
						}

						if (i == 7) {
							if ((this.capRs485)&&(typeof sensors == 'object')&&(sensors.rs485_lls_right)) {
								this.lls_right_data_field	= 'arr_rs485_2';
								this.lls_right_mask_field	= 'arr_bits1';
								this.lls_right_mask_bit		= RECORD_BIT1_RS485_2;
								this.lls_right_factor		= 1;
								i = 0;
							}
						}

						if (i <= 6) {

							sensor	= table[0].sensor_value;
							fuel	= table[0].fuel_value;
	
							for (i = 1; i < table_len; i++) 
							{	
								sensor_value	= table[i].sensor_value;
								fuel_value		= table[i].fuel_value;
	               	
								table[i - 1].d = (fuel_value - fuel) / (sensor_value - sensor);
	
								sensor	= sensor_value;
								fuel	= fuel_value;
							}

							this.lls_right_table	= table;
							this.has_lls_right		= true;
							this.has_lls			= true;
						}															
					}
				}
			}			
		}
	}

	this.lls_both_tanks	= (this.lls_left_data_field != '')&&(this.lls_right_data_field != '');
	this.lls_only_left	= (this.both_tanks) ? false : (this.lls_left_data_field != '');
	this.lls_only_right	= (this.both_tanks) ? false : (this.lls_right_data_field != '');

	this.clear();

	var last_nav = jsonObject.last_nav;
		
	if (typeof last_nav == 'object') 
	{
		var nav = {};

		nav.time	= last_nav.time;
		nav.lat		= last_nav.lat / 10000000;
		nav.lng		= last_nav.lng / 10000000;
		nav.cog		= last_nav.cog;
		nav.speed	= last_nav.speed / 10;

		this.min_lat = nav.lat;
		this.max_lat = nav.lat;
		this.min_lng = nav.lng;
		this.max_lng = nav.lng;
		
		this.last_record_time = nav.time;

		this.max_speed = nav.speed;
		this.max_speed_lng = nav.lng;
		this.max_speed_lat = nav.lat;

		this.nav = nav;
	}

	var last_data = jsonObject.last_data;
		
	if (typeof last_data == 'object') 
	{
		var data = {};

		data.time		= last_data.time;
		data.flags1		= last_data.flags1;
		data.flags2		= last_data.flags2;
		data.adc1		= last_data.adc1;
		data.adc2		= last_data.adc2;
		data.adc3		= last_data.adc3;
		data.frequency1	= last_data.frequency1;
		data.frequency2	= last_data.frequency2;
		data.frequency3	= last_data.frequency3;
		data.frequency4	= last_data.frequency4;
		data.rs485_1	= last_data.rs485_1;
		data.rs485_2	= last_data.rs485_2;

		this.data = data;
	}

	this.link = jsonObject.online;
};

Terminal.prototype.clear = function()
{ 
	this.from				= 0;
	this.to					= 0;

	this.cog				= 0;
	this.min_lat			= 90;
	this.max_lat			= -90;
	this.min_lng			= 180;
	this.max_lng			= -180;
	this.nav_points_count	= 0;
	this.last_record_time	= 0;
	this.mileage			= 0;
	this.max_speed			= 0;
	this.max_speed_lat		= 0;
	this.max_speed_lng		= 0;
	this.start_work			= 0;
	this.end_work			= 0;
	this.work_time			= 0;
	this.work_start_lat		= 0;
	this.work_start_lng		= 0;
	this.work_end_lat		= 0;
	this.work_end_lng		= 0;

	this.ignition_first		= 0;
	this.ignition_last		= 0;
	this.ignition_time		= 0;
	this.ignition_count		= 0;
	this.ignition			= false;
	this.ignition_start		= 0;
	this.ignition_first_lat	= 0;
	this.ignition_first_lng	= 0;
	this.ignition_last_lat	= 0;
	this.ignition_last_lng	= 0;

	this.engine_first		= 0;
	this.engine_last		= 0;
	this.engine_time		= 0;
	this.engine_count		= 0;
	this.engine				= false;
	this.engine_start		= 0;
	this.engine_first_lat	= 0;
	this.engine_first_lng	= 0;
	this.engine_last_lat	= 0;
	this.engine_last_lng	= 0;

	this.move_first			= 0;
	this.move_last			= 0;
	this.move_time			= 0;
	this.move_count			= 0;
	this.move				= false;
	this.move_start			= 0;
	this.move_hours			= 0;
	this.move_minutes		= 0;
	this.move_first_lat		= 0;
	this.move_first_lng		= 0;
	this.move_last_lat		= 0;
	this.move_last_lng		= 0;

	this.fuel_start			= 0;
	this.fuel_end			= 0;
	this.fuel_first			= -1;
	this.fuel_last			= -1;

	this.link				= false;
	this.mode				= 0;

	this.lastX				= 0;
	this.lastY				= 0;
	this.lastZ				= 0;

	this.last_info_point	= -1;

	this.engine_launch_count = 0;

	this.arr_time			= new Uint32Array(0);
	this.arr_bits1			= new Uint8Array(0);
	this.arr_bits2			= new Uint8Array(0);
	this.arr_bits3			= new Uint8Array(0);
	this.arr_bits4			= new Uint8Array(0);
	this.arr_bits5			= new Uint8Array(0);

	this.arr_event			= new Uint8Array(0);

	this.arr_flags1			= new Uint8Array(0);
	this.arr_flags2			= new Uint8Array(0);

	this.arr_lat			= new Float32Array(0);
	this.arr_lng			= new Float32Array(0);
	this.arr_speed			= new Uint16Array(0);

	this.track_lat			= new Float32Array(0);
	this.track_lng			= new Float32Array(0);
	this.track_index		= new Uint32Array(0);
	this.track_points_count	= 0;
	this.cog				= 0;

	this.park_lat			= new Float32Array(0);
	this.park_lng			= new Float32Array(0);
	this.park_index			= new Uint32Array(0);
	this.parks_points_count	= 0;

	for (i = 1; i <= 8; i++) 
	{
		if (this['senAnalog' + i] > 0)
			this['arr_analog' + i] = new Uint16Array(0);
		
		if (this['senFrequency' + i] > 0)
			this['arr_frequency' + i] = new Uint16Array(0);
			
		if (this['senCounter' + i] > 0)
			this['arr_counter' + i] = new Uint16Array(0);
	}

	if (this.capCog === true) {
		this.arr_cog = new Uint8Array(0);
	}

	if (this.capSatCount === true) {
		this.arr_sat_count = new Uint8Array(0);
	}

	if (this.capAltitude === true) {
		this.arr_altitude = new Uint16Array(0);
	}

	if (this.capRpm === true) {
		this.arr_rpm = new Uint16Array(0);
	}

	if (this.capRs485 === true) {
		this.arr_rs485_1 = new Uint16Array(0);
		this.arr_rs485_2 = new Uint16Array(0);
	}

	if (this.capRs232_1 === true) {
		this.arr_rs232_1 = new Uint16Array(0);
	}

	if (this.capRs232_2 === true) {
		this.arr_rs232_1 = new Uint16Array(0);
	}

	if (this.capVcc === true) {
		this.arr_vcc = new Uint16Array(0);
	}

	if (this.capInjector === true) {
		this.arr_injector = new Uint32Array(0);
	}

	if (this.capOdometr === true) {
		this.arr_odometr = new Uint32Array(0);
	}

	if (this.lls_both_tanks || this.lls_only_left) {
		this.arr_left_fuel = new Float32Array(0);
	}

	if (this.lls_both_tanks || this.lls_only_right) {
		this.arr_right_fuel = new Float32Array(0);
	}

	if (this.lls_both_tanks || this.lls_only_left || this.lls_only_right) {
		this.arr_total_fuel = new Float32Array(0);
		this.arr_total_fuel_mask = new Uint8Array(0);
	}

	this.points_count = 0;
	this.markers = [];
	this.fuel_data = null;

	this.updateFields();
}	

Terminal.prototype.updateFields = function()
{
	if (this.ignition_field != '') {
		this.ignition_data = this[this.ignition_field];
		this.ignition_mask = this[this.ignition_mask_field];
	}
	else {
		this.ignition_data = null;
		this.ignition_mask = null;
	}

	if (this.engine_field != '') {
		this.engine_data = this[this.engine_field];
		this.engine_mask = this[this.engine_mask_field];
	}
	else {
		this.engine_data = null;
		this.engine_mask = null;
	}

	if (this.move_field != '') {
		this.move_data = this[this.move_field];
		this.move_mask = this[this.move_mask_field];
	}
	else {
		this.move_data = null;
		this.move_mask = null;
	}

	if (this.lls_left_data_field != '') {
		this.lls_left_data = this[this.lls_left_data_field];
		this.lls_left_mask = this[this.lls_left_mask_field];
	}
	else {
		this.lls_left_data = null;
		this.lls_left_mask = null;
	}

	if (this.lls_right_data_field != '') {
		this.lls_right_data = this[this.lls_right_data_field];
		this.lls_right_mask = this[this.lls_right_mask_field];
	}
	else {
		this.lls_right_data = null;
		this.lls_right_mask = null;
	}
}

function get64u(data, i) {

	return data[i] + (data[i + 4] << 8) + (data[i + 8] << 16) + (data[i + 12] << 24) + (data[i + 16] << 32) + (data[i + 20] << 40) + (data[i + 24] << 48) + (data[i + 28] << 56);
}

function get32u(data, i) {

	return data[i] + (data[i + 4] << 8) + (data[i + 8] << 16) + (data[i + 12] << 24);
}

function get16u(data, i) {

	return data[i] + (data[i + 4] << 8);
}

Terminal.prototype.Uint8Concat = function(first, second, points_to_remove)
{
	var firstLength = first.length - points_to_remove,
		result = new Uint8Array(firstLength + second.length);

	result.set(first);
	result.set(second, firstLength);

	return result;
}

Terminal.prototype.Uint16Concat = function(first, second, points_to_remove)
{
	var firstLength = first.length - points_to_remove,
		result = new Uint16Array(firstLength + second.length);

	result.set(first);
	result.set(second, firstLength);

	return result;
}

Terminal.prototype.Uint32Concat = function(first, second, points_to_remove)
{
	var firstLength = first.length - points_to_remove,
		result = new Uint32Array(firstLength + second.length);

	result.set(first);
	result.set(second, firstLength);

	return result;
}

Terminal.prototype.Float32Concat = function(first, second, points_to_remove)
{
	var firstLength = first.length - points_to_remove,
		result = new Float32Array(firstLength + second.length);

	result.set(first);
	result.set(second, firstLength);

	return result;
}

Terminal.prototype.mergeHistory = function(from, to, onSuccess, onFailure) 
{	
	this.from = from;
	this.to = to;

	var self = this;

	var nav_points_count = this.nav_points_count;

	var image = new Image();
	
	image.onload = function() {
        
		var width = image.width;
		var height = image.height;

		if ((width == 1)&&(height == 1)) {
			if (onSuccess)
				onSuccess(self, 0);
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
				onFailure(self);

			return;
		}

		var arr_time	= new Uint32Array(pointsCount);
		var arr_bits1	= new Uint8Array(pointsCount);
		var arr_bits2	= new Uint8Array(pointsCount);
		var arr_bits3	= new Uint8Array(pointsCount);
		var arr_bits4	= new Uint8Array(pointsCount);
		var arr_bits5	= new Uint8Array(pointsCount);
		
		var arr_event	= new Uint8Array(pointsCount);

		var arr_flags1	= new Uint8Array(pointsCount);
		var arr_flags2	= new Uint8Array(pointsCount);

		var arr_lat		= new Float32Array(pointsCount);
		var arr_lng		= new Float32Array(pointsCount);
		var arr_speed	= new Uint16Array(pointsCount);

		var arr_cog;
		var arr_sat_count;
		var arr_altitude;
		var arr_rpm;
		var arr_rs485_1;
		var arr_rs485_2;
		var arr_rs232_1;
		var arr_rs232_2;
		var arr_vcc;
		var arr_injector;
		var arr_odometr;

		var result	= {};

		for (i = 1; i <= 8; i++) 
		{
			if (self['senAnalog' + i] !== 0)
				result['analog' + i] = new Uint16Array(pointsCount);
		
			if (self['senFrequency' + i] !== 0)
				result['frequency' + i] = new Uint16Array(pointsCount);
			
			if (self['senCounter' + i] !== 0)
				result['counter' + i] = new Uint16Array(pointsCount);
		}

		if (self.capCog === true) {
			arr_cog = new Uint8Array(pointsCount);
		}

		if (self.capSatCount === true) {
			arr_sat_count = new Uint8Array(pointsCount);
		}

		if (self.capAltitude) {
			arr_altitude = new Uint16Array(pointsCount);
		}

		if (self.capRpm) {
			arr_rpm = new Uint16Array(pointsCount);
		}

		if (self.capRs485) {
			arr_rs485_1 = new Uint16Array(pointsCount);
			arr_rs485_2 = new Uint16Array(pointsCount);
		}

		if (self.capRs232_1) {
			arr_rs232_1 = new Uint16Array(pointsCount);
		}

		if (self.capRs232_2) {
			arr_rs232_1 = new Uint16Array(pointsCount);
		}

		if (self.capVcc) {
			arr_vcc = new Uint16Array(pointsCount);
		}

		if (self.capInjector) {
			arr_injector = new Uint32Array(pointsCount);
		}

		if (self.capOdometr) {
			arr_odometr = new Uint32Array(pointsCount);
		}

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

		var iPoint = 0;
		var lat;
		var lng;
		
		var min_lat			= self.min_lat;
		var max_lat			= self.max_lat;
		var min_lng			= self.min_lng;
		var max_lng			= self.max_lng;
		var nav_time		= self.arr_time;

		var last_record_time = self.last_record_time;

		for (i = 0; i < len - 4; iPoint++) {

			var size = data[i] + (data[i + 4] << 8);

			if (size == 0xCDCD) {
				break;
			}

			var next_record = i + size * 4;

			i += 8;

			arr_time[iPoint] = get32u(data, i); i+= 16;

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

				last_record_time = arr_time[iPoint];
			}

			if (bits1 & RECORD_BIT1_NAV) {

				lat = get32u(data, i) / 10000000; i += 16;
				lng = get32u(data, i) / 10000000; i += 16;

				arr_lat[iPoint] = lat;
				arr_lng[iPoint] = lng;

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
				else {

					var nav = {};

					nav.time = arr_time[iPoint];
					
					nav.lat = lat;
					nav.lng = lng;
					nav.speed = speed / 10;

					self.nav = nav;

					nav_points_count++;

					if (lat < min_lat)
						min_lat = lat;
					if (lat > max_lat)
						max_lat = lat;
					if (lng < min_lng)
						min_lng = lng;
					if (lng > max_lng)
						max_lng = lng;
				}

				last_record_time = arr_time[iPoint];
			}

			if (bits1 & RECORD_BIT1_ALT) {
				if (self.capAltitude === true) {
					arr_altitude[iPoint] = get16u(data, i);
					self.nav.altitude = arr_altitude[iPoint];
				} 
				i+= 8;
			}

			if (bits1 & RECORD_BIT1_COG) {				
				if (self.capCog === true) {
					arr_cog[iPoint] = data[i];
					self.nav.cog = data[i];
				}
				i+= 4;
			}

			if (bits1 & RECORD_BIT1_RS485_1) {
				if (self.capRs485)
					arr_rs485_1[iPoint] = get16u(data, i); 
				i+= 8;
			}

			if (bits1 & RECORD_BIT1_RS485_2) {
				if (self.capRs485)
					arr_rs485_2[iPoint] = get16u(data, i); 
				i+= 8;
			}

			if (bits2 !== 0) {

				if (bits2 & RECORD_BIT2_ADC1) {
					if (self.senAnalog1)
						arr_analog1[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_ADC2) {
					if (self.senAnalog2)
						arr_analog2[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_ADC3) {
					if (self.senAnalog3)
						arr_analog3[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY1) {
					if (self.senFrequency1)
						arr_frequency1[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY2) {
					if (self.senFrequency2)
						arr_frequency2[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY3) {
					if (self.senFrequency3)
						arr_frequency3[iPoint] = get16u(data, i); 
					i+= 8;
				}
				if (bits2 & RECORD_BIT2_FREQUENCY4) {
					if (self.senFrequency4)
						arr_frequency4[iPoint] = get16u(data, i); 
					i+= 8;
				}

				if (bits3 !== 0) {

					if (bits3 & RECORD_BIT3_VCC) {
						if (self.capVcc)
							arr_vcc[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_SAT_NO) {
						if (self.capSatCount)
							arr_sat_count[iPoint] = data[i]; 
						i+= 4;
					}
					if (bits3 & RECORD_BIT3_ADC4) {
						if (self.senAnalog4)
							arr_analog4[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER1) {
						if (self.senCounter1)
							arr_counter1[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER2) {
						if (self.senCounter2)
							arr_counter2[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER3) {
						if (self.senCounter3)
							arr_counter3[iPoint] = get16u(data, i); 
						i+= 8;
					}
					if (bits3 & RECORD_BIT3_COUNTER4) {
						if (self.senCounter4)
							arr_counter4[iPoint] = get16u(data, i); 
						i+= 8;
					}

					if (bits4 !== 0) {

						if (bits4 & RECORD_BIT4_RS232_1) {
							if (self.capRs232_1)
								arr_rs232_1[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_RS232_2) {
							if (self.capRs232_2)
								arr_rs232_2[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_ODOMETER) {
							if (self.capOdometr)
								arr_odometr[iPoint] = get32u(data, i); 
							i+= 16;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY5) {
							if (self.senFrequency5)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY6) {
							if (self.senFrequency6)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY7) {
							if (self.senFrequency7)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}
						if (bits4 & RECORD_BIT4_FREQUENCY8) {
							if (self.senFrequency8)
								arr_frequency5[iPoint] = get16u(data, i); 
							i+= 8;
						}

						if (bits5 !== 0) {
	
							if (bits5 & RECORD_BIT5_ADC5) {
								if (self.senAnalog5)
									arr_analog5[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_ADC6) {
								if (self.senAnalog6)
									arr_analog6[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER5) {
								if (self.senCounter5)
									arr_counter5[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER6) {
								if (self.senCounter6)
									arr_counter6[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER7) {
								if (self.senCounter7)
									arr_counter7[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_COUNTER8) {
								if (self.senCounter8)
									arr_counter8[iPoint] = get16u(data, i); 
								i+= 8;
							}
							if (bits5 & RECORD_BIT5_INJECTOR) {
								if (self.capInjector)
									arr_injector[iPoint] = get32u(data, i); 
								i+= 16;
							}
						}
					}
				}
			}
			
			i = next_record;
		}

		var points_to_remove = 0;

		if (pointsCount > 0) {

			var old_arr_time = self.arr_time;

			var first_new_point_time = arr_time[0];
			
			for (i = old_arr_time.length; i--;) {
				if (old_arr_time[i] < first_new_point_time) {
					points_to_remove = old_arr_time.length - i - 1;
					break;
				}
			}
		}

		if (points_to_remove < pointsCount) {

			self.arr_time	= self.Uint32Concat(self.arr_time, arr_time, points_to_remove);
			self.arr_bits1	= self.Uint8Concat(self.arr_bits1, arr_bits1, points_to_remove);
			self.arr_bits2	= self.Uint8Concat(self.arr_bits2, arr_bits2, points_to_remove);
			self.arr_bits3	= self.Uint8Concat(self.arr_bits3, arr_bits3, points_to_remove);
			self.arr_bits4	= self.Uint8Concat(self.arr_bits4, arr_bits4, points_to_remove);
			self.arr_bits5	= self.Uint8Concat(self.arr_bits5, arr_bits5, points_to_remove);

			self.arr_event	= self.Uint8Concat(self.arr_event, arr_event, points_to_remove);

			self.arr_flags1	= self.Uint8Concat(self.arr_flags1, arr_flags1, points_to_remove);
			self.arr_flags2	= self.Uint8Concat(self.arr_flags2, arr_flags2, points_to_remove);

			self.arr_lat	= self.Float32Concat(self.arr_lat, arr_lat, points_to_remove);	
			self.arr_lng	= self.Float32Concat(self.arr_lng, arr_lng, points_to_remove);	
			self.arr_speed	= self.Uint16Concat(self.arr_speed, arr_speed, points_to_remove);

			for (i = 1; i <= 8; i++) 
			{
				if (self['senAnalog' + i] > 0)
					self['arr_analog' + i] = self.Uint16Concat(self['arr_analog' + i], result['analog' + i], points_to_remove);
		
				if (self['senFrequency' + i] > 0)
					self['arr_frequency' + i] = self.Uint16Concat(self['arr_frequency' + i], result['frequency' + i], points_to_remove);
			
				if (self['senCounter' + i] > 0)
					self['arr_counter' + i] = self.Uint16Concat(self['arr_counter' + i], result['counter' + i], points_to_remove);
			}

			if (self.capCog === true) {
				self.arr_cog = self.Uint8Concat(self.arr_cog, arr_cog, points_to_remove);
			}

			if (self.capSatCount === true) {
				self.arr_sat_count = self.Uint8Concat(self.arr_sat_count, arr_sat_count, points_to_remove);
			}

			if (self.capAltitude === true) {
				self.arr_altitude = self.Uint16Concat(self.arr_altitude, arr_altitude, points_to_remove);
			}

			if (self.capRpm === true) {
				self.arr_rpm = self.Uint16Concat(self.arr_rpm, arr_rpm, points_to_remove);
			}

			if (self.capRs485 === true) {
				self.arr_rs485_1 = self.Uint16Concat(self.arr_rs485_1, arr_rs485_1, points_to_remove);
				self.arr_rs485_2 = self.Uint16Concat(self.arr_rs485_2, arr_rs485_2, points_to_remove);
			}

			if (self.capRs232_1 === true) {
				self.arr_rs232_1 = self.Uint16Concat(self.arr_rs232_1, arr_rs232_1, points_to_remove);
			}

			if (self.capRs232_2 === true) {
				self.arr_rs232_2 = self.Uint16Concat(self.arr_rs232_2, arr_rs232_2, points_to_remove);
			}

			if (self.capVcc === true) {
				self.arr_vcc = self.Uint16Concat(self.arr_vcc, arr_vcc, points_to_remove);
			}

			if (self.capInjector === true) {
				self.arr_injector = self.Uint32Concat(self.arr_injector, arr_injector, points_to_remove);
			}

			if (self.capOdometr === true) {
				self.arr_odometr = self.Uint32Concat(self.arr_odometr, arr_odometr, points_to_remove);
			}				
 	
			self.min_lat	= min_lat;
			self.max_lat	= max_lat;
			self.min_lng	= min_lng;
			self.max_lng	= max_lng;
			self.nav_time	= nav_time;

			self.last_record_time = last_record_time;

			self.updateFields();

			var points_count		= self.arr_time.length;

			self.points_count		= points_count;
			self.nav_points_count	= nav_points_count;

			var lls_left	= self.lls_both_tanks || self.lls_only_left;
			var lls_right	= self.lls_both_tanks || self.lls_only_right;

			if (lls_left || lls_right) {

				var left_data;
				var left_mask;
				var left_bit;
				var left_table;
				var left_factor;
				var arr_left_fuel;

				var right_data;
				var right_mask;
				var right_bit;
				var right_table;
				var right_factor;
				var arr_right_fuel;

				if (lls_left) {

					left_data			= self.lls_left_data;
					left_mask			= self.lls_left_mask;
					left_bit			= self.lls_left_mask_bit;
					left_table			= self.lls_left_table;	
					left_factor			= self.lls_left_factor;

					arr_left_fuel		= new Float32Array(points_count);
					arr_left_fuel.set(self.arr_left_fuel);
				}

				if (lls_right) {

					right_data			= self.lls_right_data;
					right_mask			= self.lls_right_mask;
					right_bit			= self.lls_right_mask_bit;
					right_table			= self.lls_right_table;	
					right_factor		= self.lls_right_factor;

					arr_right_fuel		= new Float32Array(points_count);
					arr_right_fuel.set(self.arr_right_fuel);
				}
				
				var arr_total_fuel		= new Float32Array(points_count);
				arr_total_fuel.set(self.arr_total_fuel);
			
				var arr_total_fuel_mask	= new Float32Array(points_count);
				arr_total_fuel_mask.set(self.arr_total_fuel_mask);

				var fuel;

				for (var i = points_count - pointsCount; i < points_count; i++) {
				       
					arr_total_fuel_mask[i] = 255;

					if (lls_left) {
			
						if ((left_mask[i] & left_bit) === 0) {
							arr_total_fuel_mask[i] = 0;
							continue;
						}
			
			            fuel = self.SensorToFuel(left_table, left_data[i] * left_factor);

						arr_left_fuel[i] = fuel;

						arr_total_fuel[i] = fuel;
					}
			
					if (lls_right) {
				
						if ((right_mask[i] & right_bit) === 0) {
							arr_total_fuel_mask[i] = 0;
							continue;
						}
			
			            fuel = self.SensorToFuel(right_table, right_data[i] * right_factor);

						arr_right_fuel[i] = fuel;
						arr_total_fuel[i] += fuel;
					}
				}

				if (lls_left)
					self.arr_left_fuel = arr_left_fuel;

				if (lls_right)
					self.arr_right_fuel	= arr_right_fuel;

				self.arr_total_fuel			= arr_total_fuel;
				self.arr_total_fuel_mask	= arr_total_fuel_mask;
			}
	  	}
	  	else {
	  		console.log('no new points');
	  	}

		if (onSuccess)
			onSuccess(self, pointsCount - points_to_remove);
	};

	image.onerror = onFailure;

	image.src = '/history/' + this.id + '/' + from + '/' + to + '.png?tc=' + (new Date).getTime();
}


Terminal.prototype.SensorToFuel = function(table, sensor) 
{
	var table_len = table.length;

	if (table_len > 0) {

		if (sensor <= table[0].sensor_value) {
			return table[0].fuel_value;
		}
		else {

			if (sensor >= table[table_len - 1].sensor_value) {
				return table[table_len - 1].fuel_value;
			}
			else {

				var prev_item = table[table_len - 1];

				for (var j = table_len - 1; j--;) {

					var item = table[j];

					if ((sensor >= item.sensor_value)&&(sensor < prev_item.sensor_value)) {

						return item.fuel_value + item.d * (sensor - item.sensor_value);
					}


					prev_item = item;
				}
			}
		}
	}
}

Terminal.prototype.updateInfo = function() 
{
	var c_A					= 6378137;
	var c_a					= 1/298.257223563;
	var c_e2				= (2*c_a - c_a*c_a);
	var fPI					= 3.14159265358979;

	var ignition_data		= this.ignition_data;
	var ignition_bit		= this.ignition_bit;
	var ignition_first		= this.ignition_first;
	var ignition_last		= this.ignition_last;
	var ignition_time		= this.ignition_time;
	var ignition_count		= this.ignition_count;
	var ignition			= this.ignition;
	var ignition_start		= this.ignition_start;
	var ignition_first_lat	= this.ignition_first_lat;
	var ignition_first_lng	= this.ignition_first_lng;
	var ignition_last_lat	= this.ignition_last_lat;
	var ignition_last_lng	= this.ignition_last_lng;

	var engine_data			= this.engine_data;
	var engine_bit			= this.engine_bit;
	var engine_first		= this.engine_first;
	var engine_last			= this.engine_last;
	var engine_time			= this.engine_time;
	var engine_count		= this.engine_count;
	var engine				= this.engine;
	var engine_start		= this.engine_start;
	var engine_first_lat	= this.engine_first_lat;
	var engine_first_lng	= this.engine_first_lng;
	var engine_last_lat		= this.engine_last_lat;
	var engine_last_lng		= this.engine_last_lng;

	var move_data			= this.move_data;
	var move_bit			= this.move_bit;
	var move_first			= this.move_first;
	var move_last			= this.move_last;
	var move_time			= this.move_time;
	var move_count			= this.move_count;
	var move				= this.move;
	var move_start			= this.move_start;
	var move_first_lat		= this.move_first_lat;
	var move_first_lng		= this.move_first_lng;
	var move_last_lat		= this.move_last_lat;
	var move_last_lng		= this.move_last_lng;

	var fuel_first			= this.fuel_first;
	var fuel_last			= this.fuel_last;
	var lls_both_tanks		= this.lls_both_tanks;
	var lls_only_left		= this.lls_only_left;
	var lls_only_right		= this.lls_only_right;
	var lls_left_mask 		= this.lls_left_mask;
	var lls_left_bit 		= this.lls_left_mask_bit;
	var lls_right_mask 		= this.lls_right_mask;
	var lls_right_bit 		= this.lls_right_mask_bit;

	var mileage				= this.mileage;
	var link				= this.link;
	var nav_valid			= this.nav_valid;
	var mode				= this.mode;

	var max_speed			= this.max_speed;
	var max_speed_lat		= this.max_speed_lat;
	var max_speed_lng		= this.max_speed_lng;

	var arr_bits1			= this.arr_bits1;
	var arr_flags2			= this.arr_flags2;
	var arr_speed			= this.arr_speed;
	var arr_lat				= this.arr_lat;
	var arr_lng				= this.arr_lng;
	var arr_event			= this.arr_event;
	var arr_time			= this.arr_time;
	var arr_fuel			= this.arr_fuel;

	var lastX				= this.lastX;
	var lastY				= this.lastY;
	var lastZ				= this.lastZ;
		
	var points_count		= this.points_count;

	var z;

	var bit1;
	var lat;
	var lng;
	var f;

	if ((lastX === 0)&&(lastY === 0)&&(lastZ === 0)) {

		for (i = 0; i < points_count; i++) {

			if (arr_bits1[i] & 0x20) {

				var fSinL1 = Math.sin(arr_lng[i]*fPI/180);
				var fCosL1 = Math.cos(arr_lng[i]*fPI/180);
				var fSinB1 = Math.sin(arr_lat[i]*fPI/180);
				var fCosB1 = Math.cos(arr_lat[i]*fPI/180);

				var N1 = c_A/Math.sqrt (1 - c_e2*fSinB1*fSinB1);

				lastX = N1*fCosB1*fCosL1;
				lastY = N1*fCosB1*fSinL1;
				lastZ = (1 - c_e2)*N1*fSinB1;

				break;
			}
		}
	}

	for (i = this.last_info_point + 1; i < points_count; i++) {

		if (arr_event[i] > 0) {
		
			if (arr_event[i] == 1) {
				link = true;
			}
			else
			if (arr_event[i] == 2) {
				link = false;
			}
		}

		bit1 = arr_bits1[i];

		if (bit1 & 0x40) {
		
			f = arr_flags2[i];

			if (((f & 0x04) === 0)&&((f & 0x02) === 0)) {
				mode = 0;
			}
			else {
				if (((f & 0x04) !== 0)&&((f & 0x02) === 0)) {
					mode = 1;
				}
				else {
					if (((f & 0x04) === 0)&&((f & 0x02) !== 0)) {
						mode = 2;
					}
				}
			}

			if (bit1 & 0x20) {

				nav_valid = true;

			    lat = arr_lat[i];
			    lng = arr_lng[i];

				if (arr_speed[i] > max_speed) {

					max_speed		= arr_speed[i];
					max_speed_lat	= lat;
					max_speed_lng	= lng;
				}

				var fSinL2 = Math.sin(arr_lng[i]*fPI/180);
				var fCosL2 = Math.cos(arr_lng[i]*fPI/180);
				var fSinB2 = Math.sin(arr_lat[i]*fPI/180);
				var fCosB2 = Math.cos(arr_lat[i]*fPI/180);

				var N2 = c_A/Math.sqrt (1 - c_e2*fSinB2*fSinB2);

				var X2 = N2*fCosB2*fCosL2;
				var Y2 = N2*fCosB2*fSinL2;
				var Z2 = (1 - c_e2)*N2*fSinB2;

				var D = Math.sqrt((lastX - X2)*(lastX - X2) + (lastY - Y2)*(lastY - Y2) + (lastZ - Z2)*(lastZ - Z2));

				var D2 = 2*N2*Math.asin (0.5*D/N2);

				lastX = X2;
				lastY = Y2;
				lastZ = Z2;

				mileage += D2;
			}
			else {
				nav_valid = false;
			}
		
			if (ignition_data !== null) {

				z = ignition_data[i] & ignition_bit;

				if (z) {

					if (ignition_first === 0) {
						ignition_first = arr_time[i];
						ignition_first_lat = lat;
						ignition_first_lng = lng;
					}

					ignition_last = arr_time[i];
					ignition_last_lat = lat;
					ignition_last_lng = lng;
				}

				if (z != ignition) {

					ignition = z;

					if (ignition) {
						ignition_count++;
						ignition_start = arr_time[i];
					}
					else {
						ignition_time += arr_time[i] - ignition_start;
					}
				}
			}

			if (engine_data !== null) {

				z = engine_data[i] & engine_bit;

				if (z) {

					if (engine_first === 0) {
						engine_first = arr_time[i];
						engine_first_lat = lat;
						engine_first_lng = lng;
					}

					engine_last = arr_time[i];
					engine_last_lat = lat;
					engine_last_lng = lng;
				}

				if (z != engine) {

					engine = z;

					if (engine) {
						engine_count++;
						engine_start = arr_time[i];
					}
					else {
						engine_time += arr_time[i] - engine_start;								
					}
				}
			}

			if (move_data !== null) {

				z = move_data[i] & move_bit;

				if (z) {

					if (move_first === 0) {
						move_first = arr_time[i];
						move_first_lat = lat;
						move_first_lng = lng;
					}

					move_last = arr_time[i];
					move_last_lat = lat;
					move_last_lng = lng;
				}

				if (z != move) {

					move = z;

					if (move) {
						move_count++;
						move_start = arr_time[i];
					}
					else {
						move_time += arr_time[i] - move_start;								
					}
				}
			}	
		}
			
		if (((lls_both_tanks)&&(lls_left_mask[i] & lls_left_bit)&&(lls_right_mask[i] & lls_right_bit))||
			((lls_only_left)&&(lls_left_mask[i] & lls_left_bit))||
			((lls_only_right)&&(lls_right_mask[i] & lls_right_bit))) {

			if (fuel_first == -1) {

				fuel_first = i;
			}

			fuel_last = i;
		}
	}
	
	var t_to = this.to;
	var today = new Date();
	var now = today.getTime() / 1000;

	if (t_to > now) {
		t_to = now;
	}

	if (move_data !== null) {

		this.start_work		= move_first;
		this.end_work		= move_last;
		this.work_time		= move_time;
		this.work_start_lat	= move_first_lat;
		this.work_start_lng	= move_first_lng;
		this.work_end_lat	= move_last_lat;
		this.work_end_lng	= move_last_lng;

		if (move) {
			this.work_time += t_to - move_start;
		}
	}
	else {

		if (engine_data !== null) {

			this.start_work		= engine_first;
			this.end_work		= engine_last;
			this.work_time		= engine_time;
			this.work_start_lat	= engine_first_lat;
			this.work_start_lng	= engine_first_lng;
			this.work_end_lat	= engine_last_lat;
			this.work_end_lng	= engine_last_lng;

			if (engine) {
				this.work_time += t_to - engine_start;
			}
		}
		else {

			if (ignition_data !== null) {

				this.start_work		= ignition_first;
				this.end_work		= ignition_last;
				this.work_time		= ignition_time;
				this.work_start_lat	= ignition_first_lat;
				this.work_start_lng	= ignition_first_lng;
				this.work_end_lat	= ignition_last_lat;
				this.work_end_lng	= ignition_last_lng;
				
				if (ignition) {
					this.work_time += t_to - ignition_start;
				}
			}
		}
	}

	this.engine_launch_count = (engine_data !== null) ? engine_count : ignition_count;

	if (fuel_first != -1) {				

		var table;
		var sensor;

		this.fuel_start = 0;
		this.fuel_end = 0;
		
		if (lls_both_tanks || lls_only_left) {

			table = this.lls_left_table;	

			sensor = this.lls_left_data[fuel_first] * this.lls_left_factor;						
			this.fuel_start += this.SensorToFuel(table, sensor);

			sensor = this.lls_left_data[fuel_last] * this.lls_left_factor;						
			this.fuel_end += this.SensorToFuel(table, sensor);
		}


		if (lls_both_tanks || lls_only_right) {

			table = this.lls_right_table;	

			sensor = this.lls_right_data[fuel_first] * this.lls_right_factor;						
			this.fuel_start += this.SensorToFuel(table, sensor);

			sensor = this.lls_right_data[fuel_last] * this.lls_right_factor;						
			this.fuel_end += this.SensorToFuel(table, sensor);
		}
	}

	this.ignition_first		= ignition_first;
	this.ignition_last		= ignition_last;
	this.ignition_time		= ignition_time;
	this.ignition_count		= ignition_count;
	this.ignition			= ignition;
	this.ignition_start		= ignition_start;
	this.ignition_first_lat	= ignition_first_lat;
	this.ignition_first_lng	= ignition_first_lng;
	this.ignition_last_lat	= ignition_last_lat;
	this.ignition_last_lng	= ignition_last_lng;

	this.engine_first		= engine_first;
	this.engine_last		= engine_last;
	this.engine_time		= engine_time;
	this.engine_count		= engine_count;
	this.engine				= engine;
	this.engine_start		= engine_start;
	this.engine_first_lat	= engine_first_lat;
	this.engine_first_lng	= engine_first_lng;
	this.engine_last_lat	= engine_last_lat;
	this.engine_last_lng	= engine_last_lng;

	this.move_first			= move_first;
	this.move_last			= move_last;
	this.move_time			= move_time;
	this.move_count			= move_count;
	this.move				= move;
	this.move_start			= move_start;
	this.move_first_lat		= move_first_lat;
	this.move_first_lng		= move_first_lng;
	this.move_last_lat		= move_last_lat;
	this.move_last_lng		= move_last_lng;

	this.fuel_first			= fuel_first;
	this.fuel_last			= fuel_last;

	this.mileage			= mileage;
	this.link				= link;
	this.nav_valid			= nav_valid;
	this.mode				= mode;

	this.max_speed			= max_speed;
	this.max_speed_lat		= max_speed_lat;
	this.max_speed_lng		= max_speed_lng;

	this.lastX				= lastX;
	this.lastY				= lastY;
	this.lastZ				= lastZ;

	this.last_info_point	= points_count - 1;
}

Terminal.prototype.createOverlays = function(from, to, pack_parks, points_from_end) 
{
	var points_count		= this.points_count;
	var arr_lat				= this.arr_lat;
	var arr_lng				= this.arr_lng;
	var arr_time			= this.arr_time;
	var arr_bits1			= this.arr_bits1;
	var move_data			= this.move_data;
	var move_bit			= this.move_bit;
	var arr_cog				= this.arr_cog;

	var lat					= new Float32Array(points_count);
	var lng					= new Float32Array(points_count);
	var index				= new Uint32Array(points_count);

	var park				= false;
	var track_points_count	= 0;
	var park_points_count	= 0;

	for (var i = 0; i < points_count; i++) {

		if ((arr_bits1[i] & 0x20) === 0)
			continue;

		if (arr_time[i] < from)
			continue;

		if (arr_time[i] > to)
			break;

		if ((arr_bits1[i] & 0x40)&&(pack_parks === true)&&(move_data !== null)) {

			if ((move_data[i] & move_bit) === 0) {
				if (park === true)
					continue;
				park = true;
			}
			else {
				park = false;
			}
		}

		lat[track_points_count] = arr_lat[i];
		lng[track_points_count] = arr_lng[i];
		index[track_points_count] = i;

		this.cog = (typeof arr_cog == 'object') ? arr_cog[i] : 0;

		track_points_count++;
	}

	if ((typeof points_from_end == 'number')&&(points_from_end > 0)&&(points_from_end < track_points_count)) {
		
		var short_lat		= new Float32Array(points_from_end);
		var short_lng		= new Float32Array(points_from_end);
		var short_index		= new Uint32Array(points_from_end);

		for (i = 0; i < points_from_end; i++) {

			short_lat[i]	= lat[track_points_count - (points_from_end - i)];
			short_lng[i]	= lng[track_points_count - (points_from_end - i)];
			short_index[i]	= index[track_points_count - (points_from_end - i)];
		}

		track_points_count = points_from_end;

		lat = short_lat;
		lng = short_lng;
		index = short_index;
	}
	
	if (move_data !== null) {
	
		var k;
		var parks = [];

		for (i = 0; i < track_points_count; i++) {

			k = index[i];

			if ((move_data[k] & move_bit) === 0)
				parks.push(i);
		}

		var parks_len = parks.length;

		this.park_lat	= new Float32Array(parks_len);
		this.park_lng	= new Float32Array(parks_len);
		this.park_index	= new Uint32Array(parks_len);

		for (i = 0; i < parks_len; i++) {
			this.park_lat[i] = lat[parks[i]];
			this.park_lng[i] = lng[parks[i]];
			this.park_index[i] = index[parks[i]];
		}

		this.parks_points_count = parks_len;
	}
	else {
		this.park_lat	= new Float32Array(0);
		this.park_lng	= new Float32Array(0);
		this.park_index	= new Uint32Array(0);
		this.parks_points_count = 0;
	}

	this.track_lat			= lat;
	this.track_lng			= lng;
	this.track_index		= index;
	this.track_points_count	= track_points_count;

	var min_lat	= 90;
	var max_lat	= -90;
	var min_lng	= 180;
	var max_lng	= -180;

	var la;
	var ln;

	for (i = 0; i < track_points_count; i++) {

		la = lat[i];
		ln = lng[i];

		if (la < min_lat)
			min_lat = la;
		if (la > max_lat)
			max_lat = la;
		if (ln < min_lng)
			min_lng = ln;
		if (ln > max_lng)
			max_lng = ln;
	}

	this.min_lat = min_lat;
	this.max_lat = max_lat;
	this.min_lng = min_lng;
	this.max_lng = max_lng;
}

Terminal.prototype.beauty_time = function(t) {

	var date = new Date(t * 1000);

	var d = date.getDate();
	var M = date.getMonth() + 1;
	var y = date.getYear() + 1900;
	var h = date.getHours();
	var m = date.getMinutes();
	var s = date.getSeconds();

	if (d < 10) d = '0' + d;
	if (M < 10) M = '0' + M;
	if (h < 10) h = '0' + h;
	if (m < 10) m = '0' + m;
	if (s < 10) s = '0' + s;

	return d + '.' + M + '.' + y + ' ' + h + ':' + m + ':' + s;
}

Terminal.prototype.getPointTooltip = function(track_point_index)
{
	if ((track_point_index < 0)||(track_point_index >= this.track_points_count))
		return;

	var index = this.track_index[track_point_index];

	var speed = this.arr_speed[index];

	var hint = '<b>'+ this.title + '</b><br>' + 'Скорость: ' + (speed / 10) + ' км/ч<br>';

	if (this.lls_both_tanks || this.lls_only_left || this.lls_only_right) {
	 
		hint += 'Уровень топлива: ' + (this.arr_total_fuel[index]).toFixed(1) + 'л<br>';
	}

	hint += 'Время: ' + this.beauty_time(this.arr_time[index]);

	return hint;
}

Terminal.prototype.getLastPointTooltip = function()
{
	return this.getPointTooltip(this.track_points_count - 1);
}

Terminal.prototype.getParkTooltip = function(park_point_index)
{
	if ((park_point_index < 0)||(park_point_index >= this.parks_points_count))
		return;

	var index = this.park_index[park_point_index];

	var speed = this.arr_speed[index];

	var hint = '<b>'+ this.title + '</b><br>';

	if (this.lls_both_tanks || this.lls_only_left || this.lls_only_right) {
	 
		hint += 'Уровень топлива: ' + (this.arr_total_fuel[index]).toFixed(1) + 'л<br>';
	}

	hint += 'Время: ' + this.beauty_time(this.arr_time[index]) + '<br>';

	var i;
	var park_stop = -1;
	
	for (i = index; i < this.points_count; i++) {
		
		if ((this.arr_bits1[i] & 0x40)&&((this.move_data[i] & this.move_bit) !== 0)) {
			
			park_stop = i;

			break;
		}
	}

	var park_start = -1;

	for (i = index; i >= 0; i--) {
		
		if ((this.arr_bits1[i] & 0x40)&&((this.move_data[i] & this.move_bit) !== 0)) {
			
			park_start = i + 1;

			break;
		}
	}

	if ((park_start == -1)&&(park_stop == -1)) {
		hint += 'Стоянка за весь период';
	}
	else
	if ((park_start != -1)&&(park_stop != -1)) {
		hint += 'Стоянка<br>c ' + this.beauty_time(this.arr_time[park_start]) + '<br>до ' + this.beauty_time(this.arr_time[park_stop]);
	}
	else
	if ((park_start != -1)&&(park_stop == -1)) {
		hint += 'Стоянка<br>c ' + this.beauty_time(this.arr_time[park_start]) + '<br>до конца периода';
	}
	else
	if ((park_start == -1)&&(park_stop != -1)) {
		hint += 'Стоянка<br>c начала периода<br>до ' + this.beauty_time(this.arr_time[park_stop]);
	}

	return hint;
}
/*
Terminal.prototype.generateFuelData = function()
{
	var left_data		= this.lls_left_data;
	var left_mask		= this.lls_left_mask;
	var left_bit		= this.lls_left_mask_bit;
	var left_table		= this.lls_left_table;	
	var left_factor		= this.lls_left_factor;
	var lls_left		= this.lls_both_tanks | this.lls_only_left;

	var right_data		= this.lls_right_data;
	var right_mask		= this.lls_right_mask;
	var right_bit		= this.lls_right_mask_bit;
	var right_table		= this.lls_right_table;	
	var right_factor	= this.lls_right_factor;
	var lls_right		= this.lls_both_tanks | this.lls_only_right;

	var points_count	= this.points_count;

	var total_fuel_data	= new Float32Array(points_count);
	var total_fuel_mask	= new Uint8Array(points_count);
	var left_fuel_data	= new Float32Array(points_count);
	var right_fuel_data	= new Float32Array(points_count);
		
	for (var i = 0; i < points_count; i++) {
		
		left_fuel_data[i]	= 0;
		right_fuel_data[i]	= 0;

		if (lls_left) {
			
			if ((left_mask[i] & left_bit) === 0)
				continue;
			
			left_fuel_data[i] = this.SensorToFuel(left_table, left_data[i] * left_factor);
		}
			
		if (lls_right) {
			
			if ((right_mask[i] & right_bit) === 0)
				continue;
			
			right_fuel_data[i] = this.SensorToFuel(right_table, right_data[i] * right_factor);
		}

		total_fuel_data[i] = left_fuel_data[i] + right_fuel_data[i];
		total_fuel_mask[i] = 255;
	}

	this.total_fuel_data	= total_fuel_data;
	this.total_fuel_mask	= total_fuel_mask;
	this.left_fuel_data		= left_fuel_data;
	this.right_fuel_data	= right_fuel_data;
}
*/