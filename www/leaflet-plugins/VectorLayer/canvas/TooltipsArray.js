function TooltipsArrayCanvas(options)
{
    console.log('Tooltip array created');

	var default_options = {
		z_index: 6
	};

   	for (var key in options) {
   		default_options[key] = options[key];
   	}

	this.options	= default_options;

	this.tooltips	= [];
    this.xy_data	= [];
    this.xy_len		= 0;

	this.font		= '14pt Calibri';
	this.margin		= 5;

    this.canvas		= this.options.layer.canvas;
    this.ctx		= this.options.layer.renderer.ctx;
};

TooltipsArrayCanvas.prototype.addTooltip = function(tooltip) {

	var tooltips = this.tooltips;

	tooltips.push(tooltip);

	if (this.updating != true)
		this.endUpdate();

	return this;
};

TooltipsArrayCanvas.prototype.removeTooltip = function(tooltip) 
{
	var tooltips = this.tooltips;

	var i;
	while ((i = tooltips.indexOf(tooltip)) != -1) {
		tooltips.splice(i, 1);
	}

	if (this.updating !== true)
		this.endUpdate();

	return this;
};

TooltipsArrayCanvas.prototype.removeAll = function() 
{
	this.tooltips	= [];
	this.xy_len		= 0;

	return this;
};

TooltipsArrayCanvas.prototype.createTextures = function() {

	var canvas		= this.canvas;
	var ctx			= this.ctx;
	var tooltips	= this.tooltips;
	var len			= tooltips.length;

	for (var i = 0; i < len; i++) {

		var tooltip = tooltips[i];
	
	    if (typeof tooltip.canvas != 'object') {

			ctx.font				= this.font;

			var txtWidth 			= ctx.measureText(tooltip.text).width + this.margin * 2 + 2;

			var tooltip_canvas		= document.createElement("canvas");
			var tooltip_ctx			= tooltip_canvas.getContext("2d");

			tooltip_canvas.width	= txtWidth;
			tooltip_canvas.height	= 32;
	
			tooltip_ctx.beginPath();
			tooltip_ctx.moveTo(1,1);
			tooltip_ctx.lineTo(txtWidth - 1, 1);
			tooltip_ctx.lineTo(txtWidth - 1, 24);
			tooltip_ctx.lineTo(16, 24);
			tooltip_ctx.lineTo(12, 31);
			tooltip_ctx.lineTo(8, 24);
			tooltip_ctx.lineTo(1, 24);
			tooltip_ctx.lineTo(1, 1);

			tooltip_ctx.fillStyle 		= 'white';
			tooltip_ctx.fill();

			tooltip_ctx.lineWidth 		= 2;
			tooltip_ctx.strokeStyle 	= '#' + tooltip.color;
			tooltip_ctx.stroke();
  
			tooltip_ctx.fillStyle 		= 'black';

			tooltip_ctx.textBaseline	= 'middle';
			tooltip_ctx.font			= this.font;
			tooltip_ctx.fillText(tooltip.text, 1 + this.margin, 13);

			tooltip.width		= txtWidth;
			tooltip.height		= tooltip_canvas.height;
			tooltip.offset_x	= 12;
			tooltip.offset_y	= tooltip.height;
			tooltip.canvas		= tooltip_canvas;
		}        		
	}
};

TooltipsArrayCanvas.prototype.render = function() 
{
	var tooltips	= this.tooltips;
	var len			= tooltips.length;
	var xy			= this.xy_data;
	var ctx			= this.ctx;

	var scale = this.options.layer.scale;
	var translate_x = this.options.layer.translate_x;
	var translate_y = this.options.layer.translate_y;

	for (var i = 0; i < len; i++) {

		var tooltip = tooltips[i];

		ctx.drawImage(tooltips[i].canvas, xy[i * 2] * scale + translate_x - tooltip.offset_x, translate_y - xy[i * 2 + 1] * scale - tooltip.offset_y);
	}
};
                      
TooltipsArrayCanvas.prototype.lookup = function(x, y) 
{
	var tooltips	= this.tooltips;
	var len			= tooltips.length;
	var xy			= this.xy_data;
	var ctx			= this.ctx;

	var scale = this.options.layer.scale;
	var translate_x = this.options.layer.translate_x;
	var translate_y = this.options.layer.translate_y;

	var result;

	for (var i = 0; i < len; i++) {

		var tooltip = tooltips[i];

		var sx = xy[i * 2] * scale + translate_x - tooltip.offset_x;
		var sy = translate_y - xy[i * 2 + 1] * scale - tooltip.offset_y;

		if ((x >= sx)&&(x <= sx + tooltip.width)&&(y >= sy)&&(y <= sy + tooltip.height)) {

			var image_data = ctx.getImageData(x, y, 1, 1);
	
			image_data.data[0] = 0;
			image_data.data[1] = 0;
			image_data.data[2] = 0;
			image_data.data[3] = 255;
		
			ctx.putImageData(image_data, x, y);

			ctx.drawImage(tooltips[i].canvas, sx, sy);

			var image_data = ctx.getImageData(x, y, 1, 1).data;

			if ((image_data[0] != 0)||(image_data[1] != 0)||(image_data[2] != 0))
				result = i / 2;
		}
	}

	return result;
};

TooltipsArrayCanvas.prototype.getPopup = function(vertex_index) 
{
	var tooltip = this.tooltips[vertex_index];

	return tooltip.popup_text();
};                  

TooltipsArrayCanvas.prototype.beginUpdate = function() {
	
	this.updating = true;

	return this;
};

TooltipsArrayCanvas.prototype.endUpdate = function() 
{
	var tooltips		= this.tooltips;
	var tooltips_count	= tooltips.length;
	var position_index	= 0;
	var xy_data			= new Uint32Array(tooltips_count * 2);

	this.createTextures();

	for (var i = 0; i < tooltips_count; i++) {

		var tooltip = tooltips[i];

		xy_data[position_index++] = 6378137 * tooltip.lng * (Math.PI/180);
		xy_data[position_index++] = 6378137 * Math.log(Math.tan(Math.PI/4+tooltip.lat*(Math.PI/180)/2)); 
	}

	this.xy_data	= xy_data;
	this.xy_len		= position_index;

	this.updating	= false;
};