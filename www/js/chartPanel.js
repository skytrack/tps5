Ext.define('Ext.chart.Panel', {

	extend: 'Ext.Panel',
	alias: 'widget.chartpanel',             
	layout: 'fit',

	// Config
	x_data:				[],
	y_data:				[],
	mask_data:			[],
	mask_bit:			0,
	binary_bit:			0,
	binary:				false,
	factor:				1.0,

	filter_bit:			0,

	labelFont:			'12pt Calibri',
	labelHeight: 		15,

	min_grid_cell_height:40,
	min_grid_cell_width:80,

	view_start:			0,
	view_end:			0,
	margin_left:		80,
	margin_top:			10,
	margin_right:		10,
	margin_bottom:		25,

	// Local
	first_valid_point:	0,
	last_valid_point:	0,
	grid_row_range:		0.0,
	grid_cell_height:	0.0,

	client_width:		0.0,
	client_height:		0.0,

	viewport_width:		0,
	viewport_height:	0,
	viewport_right:		0,
	viewport_bottom:	0,

	x_min:				0,
	x_max:				0,
	x_axis_ratio:		0.0,
	y_axis_ratio:		0.0,
	view_x_start:		0.0,
	view_x_end:			0.0,
	visible_range_x:	0.0,
	visible_range_y:	0.0,

	context:			null,
	canvas:				null,

	mouse_down:			false,
	mouse_down_x:		0,
	mouse_down_y:		0,

	data_invalid:		false,

	constructor: function(config) {

		config.items = [{
			xtype: 'box',
			itemId: 'canvas',
			autoEl:{
				tag: 'canvas',
				height: config.Height,
				width: config.Width,
			}
		}];

		this.x_data = config.x_data || this.x_data;
		this.y_data = config.y_data || this.y_data;
		this.filter_data = config.filter_data || this.filter_data;
		this.filter_bit = config.filter_bit || this.filter_bit;
		this.mask_data = config.mask_data || this.mask_data;
		this.mask_bit = config.mask_bit || this.mask_bit;
		this.binary_bit = config.binary_bit || this.binary_bit;
		this.binary = config.binary || this.binary;
		this.factor = config.factor || this.factor;
		this.labelFont = config.labelFont || this.labelFont;
		this.labelHeight = config.labelHeight || this.labelHeight;
		this.min_grid_cell_width = config.min_grid_cell_width || this.min_grid_cell_width;
		this.min_grid_cell_height = config.min_grid_cell_height || this.min_grid_cell_height;
		this.margin_left = config.margin_left || this.margin_left;
		this.margin_top = config.margin_top || this.margin_top;
		this.margin_right = config.margin_right || this.margin_right;
		this.margin_bottom = config.margin_bottom || this.margin_bottom;
		this.view_start = config.view_start || this.view_start;
		this.view_end = config.view_end || this.view_end;
		
		this.callParent(arguments);

        this.addEvents('sync');
        this.addEvents('click');

        this.on('afterrender', this.afterrender);
		this.on('resize', this.resize);

		this.invalidateData();
	},

	invalidateData: function() {

		if (typeof this.x_data == 'undefined') {
			this.x_data = [];
			this.y_data = [];
			this.mask_data = [];
			this.bit = 0;
			this.factor = 1.0;
		}

		var x_data		= this.x_data;
		var y_data		= this.y_data;
		var mask_data	= this.mask_data;
		var mask_bit	= this.mask_bit;
		var factor		= this.factor;

		var data_len = x_data.length;

		var i;
		var first_valid_point = -1;
		var last_valid_point = -1;

		for (i = 0; i < data_len; i++) {

			if (mask_data[i] & mask_bit) {
				first_valid_point = i;
				break;
			}
		}

		if (i == data_len) {
			this.y_max = 1;
			this.visible_range_y = 1;
		}
		else {

			for (i = data_len; i--;) {

				if (mask_data[i] & mask_bit) {
					last_valid_point = i;
					break;
				}
			}

			if (this.binary) {

				this.visible_range_y = 1;
			}
			else {

				var y_max = y_data[this.first_valid_point] * factor;

				for (i = first_valid_point + 1; i <= last_valid_point; i++) {

					if (mask_data[i] & mask_bit) {
			
						if (y_data[i] * factor > y_max)
							y_max = y_data[i] * factor;
					}
				}
	    	
				this.visible_range_y = y_max;
			}
		}

		this.x_min = (this.view_start) ? this.view_start : ((first_valid_point != -1) ? x_data[first_valid_point] : 0);
		this.x_max = (this.view_end) ? this.view_end : ((last_valid_point != -1) ? x_data[last_valid_point] : 0);

		this.visible_range_x = this.x_max - this.x_min;
		this.first_valid_point = first_valid_point;
		this.last_valid_point = last_valid_point;
		this.view_x_start = this.x_min;
		this.view_x_end = this.x_max;
	},

	afterrender: function(component) {

		var canvas			= component.down('#canvas').getEl().dom;
		var context			= canvas.getContext("2d");                                       

		canvas.ownerCt		= component;

		canvas.onmousedown	= component.onmousedown;
		canvas.onmouseup	= component.onmouseup;
		canvas.onmousemove	= component.onmousemove;
		canvas.ondblclick	= component.ondblclick;
		canvas.onclick		= component.onclick;
		canvas.ontouchmove	= component.ontouchmove;
		canvas.onmouseout	= component.onmouseout;

		component.canvas	= canvas;
		component.context	= context;
	},

	onmousedown: function(e) {

		var component	= this.ownerCt;
		var rect		= component.canvas.getBoundingClientRect();

		var mouse_down_x = (e.pageX - rect.left);
		var mouse_down_y = (e.pageY - rect.top);

		if ((mouse_down_x > component.margin_left)&&(mouse_down_x < component.viewport_right)&&
			(mouse_down_y > component.margin_top)&&(mouse_down_y < component.viewport_bottom)) {

			component.mouse_down = true;

			component.mouse_down_x = mouse_down_x;
			component.mouse_down_y = mouse_down_y;
		}

		return false;
	},

	onmouseup: function(e) {
		this.ownerCt.mouse_down = false;
	},

	onmousemove: function(e) {             

		var component = this.ownerCt;

		var rect = component.canvas.getBoundingClientRect();

		var x = (e.pageX - rect.left);
		var y = (e.pageY - rect.top);

		if (component.timeout) {
			clearTimeout(component.timeout);
			delete component.timeout;		
		}

		if (component.hint_timeout) {
			clearTimeout(component.hint_timeout);
			delete component.hint_timeout;		
		}

		if (component.popup) {
			component.redrawStatic(component);
			component.redrawDynamic(component);
			delete component.popup;
		}

		if (component.mouse_down) {

			var dX = component.mouse_down_x - x;
			var fX = dX * component.x_axis_ratio;

			component.view_x_start += fX;
			component.view_x_end += fX;

			if (component.view_x_start < component.x_min) {
				component.view_x_start = component.x_min;
				component.view_x_end = component.view_x_start + component.visible_range_x;
			}
			if (component.view_x_end > component.x_max) {
				component.view_x_end = component.x_max;
				component.view_x_start = component.view_x_end - component.visible_range_x;
			}

			component.mouse_down_x = x;
			component.mouse_down_y = y;

			component.redrawStatic(component);
			component.redrawDynamic(component);

			component.fireEvent('sync', component, component.view_x_start, component.view_x_end);

			component.ignore_click = true;

			return false;
		}
		else {

			var context	= component.context;

			if ((x > component.margin_left)&&(x < component.viewport_right)) {

			    var p = context.createImageData(1,1);

			    p.data[0] = 0;
			    p.data[1] = 0;
			    p.data[2] = 0;
			    p.data[3] = 255;

				if (component.cursor_data) {
					context.putImageData(component.cursor_data, component.cursor_xpos, component.margin_top);
				}

				var bottom = component.viewport_bottom;

				component.cursor_data = context.getImageData(x, component.margin_top, 1, bottom - component.margin_top);
				component.cursor_xpos = x;

				component.timeout = setTimeout(function () { component.dopopup(component, x, y); }, 1500);
				if (component.binary !== true) {
					component.hint_timeout = setTimeout(function () { component.dohint(component, x, y); }, 100);
				}

				for (var yy = component.margin_top; yy < bottom - 1; yy++) {
					context.putImageData(p,x,yy);
				}
			}
			else {
				if (component.cursor_data) {
					context.putImageData(component.cursor_data, component.cursor_xpos, component.margin_top);
					delete component.cursor_data;
				}
			}
		}
	},

	onmouseout: function(e) {

		var component = this.ownerCt;

		if (component.cursor_data) {
			component.context.putImageData(component.cursor_data, component.cursor_xpos, component.margin_top);
			delete component.cursor_data;
		}

		if (component.timeout) {
			clearTimeout(component.timeout);
			delete component.timeout;		
		}

		if (component.hint_timeout) {
			clearTimeout(component.hint_timeout);
			delete component.hint_timeout;		
		}

		if (component.popup) {
			component.redrawStatic(component);
			component.redrawDynamic(component);
			delete component.popup;
		}
	},

	ondblclick: function(e) {

		var component	= this.ownerCt;

		var rect		= component.canvas.getBoundingClientRect();
		
		var mouse_down_x = (e.pageX - rect.left);
		var mouse_down_y = (e.pageY - rect.top);

		if ((mouse_down_x > component.margin_left)&&(mouse_down_x < component.viewport_right)&&
			(mouse_down_y > component.margin_top)&&(mouse_down_y < component.viewport_bottom)) {

			// Точка во вьюпорте куда нажали
			var clickX = mouse_down_x - component.margin_left;
			var percent = clickX / component.viewport_width;

			// Время точки куда нажали
			var clickTime = component.view_x_start + (component.view_x_end - component.view_x_start) * percent;

			var visible_range_x = component.visible_range_x / 2; 

			// Увеличение вдвое, точка X в центре
			component.view_x_start = clickTime - visible_range_x * percent;
			component.view_x_end = clickTime + visible_range_x * (1 - percent);
			
			// Пересчет коэффициентов
			component.x_axis_ratio = visible_range_x / component.viewport_width;
			
			component.visible_range_x = visible_range_x;

			component.redrawStatic(component);
			component.redrawDynamic(component);

			component.fireEvent('sync', component, component.view_x_start, component.view_x_end);
		}
	},

	onclick: function(e) {

		var component	= this.ownerCt;

		var rect		= component.canvas.getBoundingClientRect();

        var mouse_down_x = (e.pageX - rect.left);
        var mouse_down_y = (e.pageY - rect.top);

        if ((mouse_down_x > component.margin_left)&&(mouse_down_x < component.viewport_right)&&
            (mouse_down_y > component.margin_top)&&(mouse_down_y < component.viewport_bottom)) {

			// Точка во вьюпорте куда нажали
			var clickX = mouse_down_x - component.margin_left;
			var percent = clickX / component.viewport_width;

			// Время точки куда нажали
			var clickTime = component.view_x_start + (component.view_x_end - component.view_x_start) * percent;

			if ((component.popup)&&(component.popup.popup)) {
				component.fireEvent('click', component, clickTime);
			}
		}
		else {

			var visible_range_x = component.visible_range_x;

			var center_x = (component.view_x_start + component.view_x_end) / 2;

			component.view_x_start = center_x - visible_range_x;
			component.view_x_end = center_x + visible_range_x;

			if (component.view_x_start < component.x_min)
				component.view_x_start = component.x_min;
					
			if (component.view_x_end > component.x_max)
				component.view_x_end = component.x_max;

			// Пересчет коэффициентов
			component.visible_range_x = component.view_x_end - component.view_x_start;
			component.x_axis_ratio = component.visible_range_x / component.viewport_width;
			
			component.redrawStatic(component);
			component.redrawDynamic(component);

			component.fireEvent('sync', component, component.view_x_start, component.view_x_end);
		}
	},

	dopopup: function(component, x, y) {

		var context = component.context;

		var label = "На карту";

		var txtSize = context.measureText(label);

       	component.popup = { 
       		x: x - txtSize.width, 
       		y: y, 
       		w: txtSize.width + 10, 
       		h: 25,
       		popup: true
       	};

		context.beginPath();
		context.rect(component.popup.x, component.popup.y, component.popup.w, component.popup.h);
		context.fillStyle = 'white';
		context.fill();
		context.lineWidth = 1;
		context.strokeStyle = 'black';
		context.stroke();
      
		context.fillStyle = 'black';
		context.fillText(label, x - txtSize.width + 5, y + 12);
	},

	dohint: function(component, x, y) {

        if ((x > component.margin_left)&&(x < component.viewport_right)&&
            (y > component.margin_top)&&(y < component.viewport_bottom)) {

			// Точка во вьюпорте куда нажали
			var clickX = x - component.margin_left;
			var percent = clickX / component.viewport_width;

			// Время точки куда нажали
			var clickTime = component.view_x_start + (component.view_x_end - component.view_x_start) * percent;

			var mask_data = component.mask_data;
			var mask_bit = component.mask_bit;

			var x_data = component.x_data;

			var diff = 0xFFFFFFFF;

			for (var i = 0; i < x_data.length; i++) {
				if (mask_data[i] & mask_bit) {
					diff = Math.abs(x_data[i] - clickTime);
					break;
				}
			}

			if (x_data.length > 0) {

				var xx = 0;

				if (clickTime < x_data[0]) {
					for (var i = 0; i < x_data.length; i++) {

						if (mask_data[i] & mask_bit) {
							xx = i;
							break;
						}
					}
				}
				else
				if (clickTime > x_data[x_data.length - 1]) {

					for (var i = x_data.length; i--;) {

						if (mask_data[i] & mask_bit) {
							xx = i;
							break;
						}
					}
				}
				else {
					for (var i = x_data.length; i--;) {
						if ((mask_data[i] & mask_bit)&&(Math.abs(x_data[i] - clickTime) < diff)) {
							xx = i;
							diff = Math.abs(x_data[i] - clickTime);
						}
					}
				}

				var context = component.context;

				var label = (component.y_data[xx] * component.factor).toFixed(1);

				var txtSize = context.measureText(label);

  		     	component.popup = { 
  	  	   			x: x - txtSize.width - 10, 
    	   			y: y, 
       				w: txtSize.width + 10, 
	       			h: 25,
	       			hint: true
	    	   	};

				context.beginPath();
				context.rect(component.popup.x, component.popup.y, component.popup.w, component.popup.h);
				context.fillStyle = 'white';
				context.fill();
				context.lineWidth = 1;
				context.strokeStyle = 'black';
				context.stroke();
      
				context.fillStyle = 'black';
				context.fillText(label, x - txtSize.width - 5, y + 12);
			}
		}
	},

	ontouchmove: function(e) {
		e.preventDefault(); 
	},

	setViewPort: function(view_x_start, view_x_end) {

		this.view_x_start = view_x_start;
		this.view_x_end = view_x_end;

		this.visible_range_x = view_x_end - view_x_start;
		this.x_axis_ratio = this.visible_range_x / this.viewport_width;

		this.redrawStatic(this);
		this.redrawDynamic(this);
	},

	applyFilter: function(filter, bit) {

		this.filter_data = filter;
		this.filter_bit = bit;
		
		this.redrawStatic(this);
		this.redrawDynamic(this);
	},

	resize: function(component, width, height) {
		
		component.canvas.setAttribute('width', width);	
		component.canvas.setAttribute('height', height);	

		component.client_width		= width;
		component.client_height		= height;

		// Границы области графика
		component.viewport_width	= width  - component.margin_left - component.margin_right;
		component.viewport_height	= height - component.margin_top  - component.margin_bottom;
		component.viewport_right	= width  - component.margin_right;
		component.viewport_bottom	= height - component.margin_bottom;

		// По оси X: одна точка на экране - это x_axis_ratio секунд данных.
		component.x_axis_ratio = component.visible_range_x / component.viewport_width;
		// По оси Y: одна точка на экране - это y_axis_ratio пунктов данных.
		component.y_axis_ratio = component.visible_range_y / (component.viewport_height - 5);

		if (component.binary) {
			component.grid_row_range = 1;
		}
		else {	
			// Минимальная высота строки сетки в пунктах данных
			var min_grid_row_range = component.min_grid_cell_height * component.y_axis_ratio;

			var step = 0.0;

			if (min_grid_row_range >= 1) {
				var fract = min_grid_row_range.toString().split(".")[0];
				step = Math.pow(10, fract.length - 1);
			}
			else
			if (min_grid_row_range > 0.1)
				step = 0.05;
			else
			if (min_grid_row_range > 0.01)
				step = 0.005;
			else
			if (min_grid_row_range > 0.001)
				step = 0.0005;

			component.grid_row_range = Math.round(min_grid_row_range / step) * step;

			if (component.grid_row_range < step)
				component.grid_row_range += step;
		}

		component.grid_cell_height = component.grid_row_range / component.y_axis_ratio;

		component.redrawStatic(component);
		component.redrawDynamic(component);
	},

	redrawStatic: function(component) {

		var viewport_bottom		= component.viewport_bottom;
		var viewport_height		= component.viewport_height;
		var margin_left			= component.margin_left;

		var grid_row_range 		= component.grid_row_range;
		var grid_cell_height	= component.grid_cell_height;

		var context				= component.context;
		var local_pow			= Math.pow;

       	// Очистка
		context.fillStyle		= 'white';
		context.fillRect(0, 0, component.client_width, component.client_height);

		if (!component.binary) {

	       	// Вертикальные метки
			context.fillStyle		= 'black';
			context.textBaseline	= 'middle';
			context.font			= component.labelFont;

			var label_value = 0.0;

			for (var y = 0; y < viewport_height; y+= grid_cell_height) {

				var label = Math.round(label_value*local_pow(10,5))/local_pow(10,5);

				context.fillText(label,
					margin_left - context.measureText(label).width - 7, 
					viewport_bottom - y);

				label_value += grid_row_range;
			}
		}
	},

	redrawDynamic: function(component) {

		var context				= component.context;
		var x_data				= component.x_data;
		var y_data				= component.y_data;
		var factor				= component.factor;
		var mask_data			= component.mask_data;
		var mask_bit			= component.mask_bit;
		var binary_bit			= component.binary_bit;
		var binary				= component.binary;
		var filter_data			= component.filter_data;
		var filter_bit			= component.filter_bit;

		var x_axis_ratio		= component.x_axis_ratio;

		var viewport_right		= component.viewport_right;
		var viewport_bottom		= component.viewport_bottom;
		var viewport_width		= component.viewport_width;
		var viewport_height		= component.viewport_height;

		var margin_left			= component.margin_left;
		var margin_top			= component.margin_top;
		var margin_bottom		= component.margin_bottom;

		var view_x_start		= component.view_x_start;
		var view_x_end			= component.view_x_end;

		var first_valid_point	= component.first_valid_point;
		var last_valid_point	= component.last_valid_point;

		var min_grid_cell_width	= component.min_grid_cell_width;

		var labelHeight			= component.labelHeight;
		var labelFont			= component.labelFont;

		var lingrad = context.createLinearGradient(margin_left, margin_top, viewport_width, viewport_height);

		lingrad.addColorStop(0.0, '#D4D4D4');
		lingrad.addColorStop(0.2, '#fff');
		lingrad.addColorStop(0.8, '#fff');
		lingrad.addColorStop(1.0, '#D4D4D4');

		context.fillStyle = lingrad;
		context.fillRect(margin_left, margin_top, viewport_width, viewport_height);
		
		context.lineWidth = 1;
		context.strokeStyle = "black";

		context.textBaseline = 'middle';

		// График

		var pre_visible_point = -1;
		var first_visible_point = -1;

		var i;

		// Находим первую видимую точку в окне просмотре.
		// Поскольку конкретно во время начала окна просмотра графика скорее всего точки нет,
		// а рисовать линию надо именно с начала - нужно найти последнюю невидимую точку перед видимой,
		// Рисовать линию небходимо из точки пересечения отрезка [невидимая точка, видимая точка] c 
		// левой границей окна просмотра

		for (i = first_valid_point; i <= last_valid_point; i++) {

			if (mask_data[i] & mask_bit) {

				if (x_data[i] < view_x_start) {
					pre_visible_point = i;
				}
				else {
					first_visible_point = i;
					break;
				}
			}
		}

		if (binary) {

			var high_y = margin_top + 10;
			var low_y  = viewport_bottom - 10;

			context.beginPath();
			context.moveTo(margin_left, viewport_bottom);
			context.lineTo(viewport_right, viewport_bottom);
			context.stroke();

			context.beginPath();

			// Начальная позиция
			// Если есть точки до начала окна просмотра - рисую график от начала просмотра
			// Иначе выставляю начало на первую видимую точку если таковая имеется

			var prev_y;			
			if (pre_visible_point != -1) {
				i = pre_visible_point + 1;
				prev_y = y_data[pre_visible_point];

				context.moveTo(margin_left, (prev_y & binary_bit) ? high_y : low_y);
			}
			else
			if (first_visible_point != -1) {
					
				prev_y = y_data[first_visible_point];

				context.moveTo(margin_left + (x_data[first_visible_point] - view_x_start) / x_axis_ratio, (prev_y & binary_bit) ? high_y : low_y);
			}

			// Основной график
			if (first_visible_point != -1) {
	
	        	var prev_x;

				for (i = first_visible_point; i <= last_valid_point; i++) {

					if (x_data[i] > view_x_end)
						break;

					if (mask_data[i] & mask_bit) {

						var y = y_data[i];

						prev_x = x_data[i];

						if (y != prev_y) {

							var x = margin_left + (prev_x - view_x_start) / x_axis_ratio;
						
							context.lineTo(x, (prev_y & binary_bit) ? high_y : low_y);
							context.lineTo(x, (y & binary_bit) ? high_y : low_y);
							
							prev_y = y;
						}
					}
				}

				context.lineTo(margin_left + (prev_x - view_x_start) / x_axis_ratio, (prev_y & binary_bit) ? high_y : low_y);

			}

			// Конечная позиция, продлеваю текущую линию до конца если дальше есть данные
			for (; i <= last_valid_point; i++) {

				if (mask_data[i] & mask_bit) {

					context.lineTo(viewport_right, (prev_y & binary_bit) ? high_y : low_y);

					break;
				}
			}

			context.stroke();
		}
		else {

			var grid_cell_height = component.grid_cell_height;
			var grid_row_range	= component.grid_cell_height;
			var y_axis_ratio	= component.y_axis_ratio;

			// Горизонтальные линии
			for (i = viewport_bottom; i < margin_top; i -= grid_cell_height) {

				context.beginPath();
				context.moveTo(margin_left, i);
				context.lineTo(viewport_right, i);
				context.stroke();
			}

			var filtering = false;

			context.beginPath();

			// Начальная позиция
			// Если есть точки до начала окна просмотра - рисую график от начала просмотра
			// Иначе выставляю начало на первую видимую точку если таковая имеется
			
			var x;
			var y;

			if (pre_visible_point != -1) {

				if ((filter_data)&&((filter_data[pre_visible_point] & filter_bit) === 0)) {
					context.moveTo(margin_left, viewport_bottom - y / y_axis_ratio);
					filtering = true;
				}
				else {

					x = x_data[pre_visible_point];
					y = y_data[pre_visible_point] * factor;

					for (i = pre_visible_point + 1; i <= last_valid_point; i++) {
	
						if (mask_data[i] & mask_bit) {

							// Производная
							var derivative = (y_data[i] * factor - y) / (x_data[i] - x);

							y += derivative * (view_x_start - x);

							break;
						}
					}
					
					context.moveTo(margin_left, viewport_bottom - y / y_axis_ratio);
				}
				x = view_x_start;
				i = pre_visible_point + 1;
			}
			else
			if (first_visible_point != -1) {
					
				x = x_data[first_visible_point];

				if ((filter_data)&&((filter_data[first_visible_point] & filter_bit) === 0)) {
					y = 0;
					filtering = true;
				}
				else {
					y = y_data[first_visible_point] * factor;
				}


				context.moveTo(margin_left + (x - view_x_start) / x_axis_ratio, viewport_bottom - y / y_axis_ratio);
			}

			// Основной график
			if (first_visible_point != -1) {

				for (i = first_visible_point; i <= last_valid_point; i++) {

					if (x_data[i] > view_x_end)
						break;
			
					if (mask_data[i] & mask_bit) {

						if ((filter_data)&&((filter_data[i] & filter_bit) === 0)) {

							if (filtering == false) {
								context.lineTo(margin_left + (x - view_x_start) / x_axis_ratio, viewport_bottom);
								filtering = true;
							}
						}
						else {
							
							x = x_data[i];
							y = y_data[i] * factor;

							if (filtering) {
								context.lineTo(margin_left + (x - view_x_start) / x_axis_ratio, viewport_bottom);
								filtering = false;
							}

							context.lineTo(margin_left + (x - view_x_start) / x_axis_ratio, viewport_bottom - y / y_axis_ratio);
						}
					}
				}
			}

			// Конечная позиция, продлеваю текущую линию до конца если дальше есть данные
			for (; i <= last_valid_point; i++) {

				if (mask_data[i] & mask_bit) {

					if (filtering) {
						context.lineTo(viewport_right, viewport_bottom);
					}
					else {					

						// Производная
						var derivative = (y_data[i] * factor - y) / (x_data[i] - x);

						y += derivative * (view_x_end - x);

						context.lineTo(viewport_right, viewport_bottom - y / y_axis_ratio);
					}

					break;
				}
			}

			context.stroke();
		}

		// Горизонтальные метки и линии
		context.font = component.labelFont;

		var yPos = viewport_bottom + labelHeight / 2 + 5;

		context.fillStyle = 'white';
		context.fillRect(margin_left, viewport_bottom, viewport_width, labelHeight + 5);

		context.fillStyle = 'black';

		for (var x = margin_left + min_grid_cell_width; (x + (min_grid_cell_width / 2)) < viewport_right; x += min_grid_cell_width * 1.5) {

			var label_time = view_x_start + (view_x_end - view_x_start) * ((x - margin_left) / viewport_width);

			var date = new Date(label_time * 1000);

			var d = date.getDate();
			var M = date.getMonth() + 1;
			var h = date.getHours();
			var m = date.getMinutes();
			var s = date.getSeconds();

			var label_value = ((d < 10)?'0':'') + d + '.' + ((M < 10)?'0':'') + M +' / '+ ((h < 10)?'0':'') + h +':'+ ((m < 10)?'0':'') + m + ':' + ((s < 10)?'0':'') + s;

			// Метка
			var txtSize = context.measureText(label_value);
			context.fillText(label_value, x - txtSize.width / 2, yPos);        		

			// Линия
			context.beginPath();
			context.moveTo(x, margin_top);
			context.lineTo(x, viewport_bottom);
			context.stroke();
		}
	}

}); // define ChartPanelClass