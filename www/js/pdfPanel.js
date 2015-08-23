Ext.define('Ext.pdf.Panel', {

	extend:	'Ext.Panel',
	alias:	'widget.pdfpanel',             
	layout:	'fit',

	constructor: function(config) {

		config.items = [{
			xtype: 'box',
			itemId: 'pdf_iframe',
			autoEl:{
				tag: 'iframe',
				height: config.Height,
				width: config.Width
			}
		}];

		this.json = config.json;

		this.callParent(arguments);

        this.on('afterrender', this.afterrender);
		this.on('resize', this.resize);

	},

	afterrender: function(component) {

		var iframe = component.down('#pdf_iframe').getEl().dom;

		iframe.contentWindow.document.write("<html><body><form method='post' action='/report.pdf'><input type='hidden' name='report' value='"+Ext.encode(component.json)+"'><input id='button' type='submit'></form></body></html>");
		iframe.contentWindow.document.getElementById("button").click();
		iframe.contentWindow.document.getElementById("button").style.visibility='hidden';

		component.iframe = iframe;
	},

	resize: function(component, width, height) {
		
		component.iframe.setAttribute('width', width);	
		component.iframe.setAttribute('height', height);	
	}
});

