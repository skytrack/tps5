/*
 * File: app.js
 *
 * This file was generated by Sencha Architect version 2.2.3.
 * http://www.sencha.com/products/architect/
 *
 * This file requires use of the Ext JS 4.2.x library, under independent license.
 * License of Sencha Architect does not include license for Ext JS 4.2.x. For more
 * details see http://www.sencha.com/license or contact license@sencha.com.
 *
 * This file will be auto-generated each and everytime you save your project.
 *
 * Do NOT hand edit this file.
 */

//@require @packageOverrides
Ext.Loader.setConfig({
	enabled: true
});

Ext.application({
	models: [
		'Auth'
	],
	stores: [
		'Devices',
		'Retranslators',
		'DiscreteSensorTypes',
		'CounterSensorTypes',
		'FrequencySensorTypes',
		'ADCSensorTypes',
		'LLSStore',
		'ReportTypes',
		'ChannelsMemoryStore',
		'FilterTypes',
		'TimeSeriesStore'
	],
	views: [
		'loginWindow',
		'MainWindow',
		'UserWindow',
		'TerminalWindow',
		'LLSForm',
		'ChartWindow',
		'GroupWindow',
		'ChannelsWindow',
		'TimeSeriesWindow'
	],
	autoCreateViewport: true,
	name: 'MyApp',

	createJWindow: function(config) {
		var conditions = [];

		function processAction(action)
		{
			var component = Ext.ComponentQuery.query('#' + action.item)[0]

			if ((action.command == "hide")&&(typeof component != 'undefined'))
			component.hide();
			if ((action.command == "show")&&(typeof component != 'undefined'))
			component.show();
			if ((action.command == "disable_item_value")&&(typeof component != 'undefined')) {

				var store = component.getStore();

				if (store) {

					store.clearFilter();

					var item = store.findRecord("id", action.value);

					if (item) {

						item.set('hidden', true);

						store.filter([
						{filterFn: function(item) { return item.get("hidden") != true; }}
						]);
					}

					if (component.getValue() == item.get('id'))
					component.select(0);
				}		
			}	
			if ((action.command == "enable_item_value")&&(typeof component != 'undefined')) {

				var store = component.getStore();

				if (store) {

					store.clearFilter();

					var item = store.findRecord("id", action.value);

					if (item) {

						item.set('hidden', false);

						store.filter([
						{filterFn: function(item) { return item.get("hidden") != true; }}
						]);
					}
				}		
			}	
		}

		function processActions(actions, selector)
		{
			for (var key in actions) {

				var action = actions[key];

				if (action.selector == selector)
				processAction(action);
			}
		}

		function processOperation(operation) 
		{
			var operand1 = operation.operand1;
			var operand1_value;

			if (operand1.operation)
			operand1_value = processOperation(operand1.operation);
			else
			if (operand1.item) {
			operand1_value = (Ext.ComponentQuery.query('#' + operand1.item)[0]).getValue();
			}
			else 
			operand1_value = operand1;

			var operand2 = operation.operand2;
			var	operand2_value;

			if (operand2.operation)
			operand2_value = processOperation(operand2.operation);
			else								
			if (operand2.item) {
				operand2_value = (Ext.ComponentQuery.query('#' + operand2.item)[0]).getValue();
			}
			else 
			operand2_value = operand2;


			if (operation.operator == 'or')
			return operand1_value | operand2_value;

			if (operation.operator == 'and')
			return operand1_value && operand2_value;

			if (operation.operator == 'equal')
			return operand1_value == operand2_value;

			if (operation.operator == 'less')
			return operand1_value < operand2_value;

			if (operation.operator == 'greater')
			return operand1_value > operand2_value;
		}

		function processConditions() 
		{
			for (var key in conditions) {

				var condition = conditions[key];

				console.log(condition);

				var selector = processOperation(condition.operation);

				console.log(selector);

				var actions = condition.actions;

				processActions(actions, selector);
			}					
		}

		function add_item_to_array(item)
		{	
			if (item.type == 'group') {

				var items = [];

				var subitems = item.items;

				for (var key in subitems)
				items.push(add_item_to_array(subitems[key]));

				var config = { items: items };

				if (item.id) {
					config.itemId = item.id;
					config.checkboxName = item.id;	
				}

				if (item.label)
				config.title = item.label;

				if (item.collapsible) {
					config.collapsible = item.collapsible;
					config.checkboxToggle = true;
					config.collapsed = !item.expanded;
				}

				return Ext.create("Ext.form.FieldSet", config);
			}

			if (item.type == 'checkbox') {

				var config = {

					sourceItem: item,						
					listeners: {
						change: {
							fn: function(checkbox, newValue, oldValue, eOpts ) {

								processConditions();

								var selector;

								if (newValue)
								selector = 'check';
								else
								selector = 'uncheck';

								var item = checkbox.sourceItem;

								var actions = item.actions;

								processActions(actions, selector);											
							}
						}
					}
				};

				if (item.id) {
					config.itemId = item.id;					
					config.name = item.id;	
				}
				if (item.label)
				config.boxLabel = item.label;					
				if (item.checked)
				config.checked = item.checked;

				return Ext.create("Ext.form.field.Checkbox", config);
			}

			if (item.type == 'text') {

				var config = {};

				if (item.id) {
					config.itemId = item.id;					
					config.name = item.id;					
				}
				if (item.label)
				config.fieldLabel = item.label;					
				if (item.minlen)
				config.minLength = item.minlen;
				if (item.maxlen)
				config.maxLength = item.maxlen;
				if (item.size)
				config.size = item.size;
				if (item.empty)
				config.emptyText = item.empty;
				config.labelWidth = 250;


				if (typeof item.value != 'undefined')
				config.value = item.value;

				if (typeof item.value == 'number')
				return Ext.create("Ext.form.field.Number", config);

				return Ext.create("Ext.form.field.Text", config);
			}


			if (item.type == 'select') {

				var states = Ext.create('Ext.data.Store', {
					fields: ['id', 'label', 'hidden'],
					data: item.items
				});

				var config = {
					store: states,
					queryMode: 'local',
					displayField: 'label',
					valueField: 'id',
					editable: false,
					sourceItem: item,
					listeners: {
						select: {
							fn: function(combo, records, eOpts) {

								processConditions();

								var item = combo.sourceItem;

								var id = records[0].getId();

								for (var key in item.items) {

									if (item.items[key].id == id) {

										var actions = item.items[key].actions;

										processActions(actions, 'select');
									}
								}
							}
						}
					}
				};

				config.labelWidth = 250;

				if (item.id) {
					config.itemId = item.id;					
					config.name = item.id;	
				}

				if (item.label)
				config.fieldLabel = item.label;					

				if (item.size)
				config.size = item.size;

				config.value = item.value;

				var cb = Ext.create("Ext.form.field.ComboBox", config);

				return cb;
			}
		}

		var json = config.json;

		var items = [];

		for (var key in json) {

			var section = json[key];

			if (section.type == 'condition')
			conditions.push(section);
			else
			items.push(add_item_to_array(section));
		}

		var panel = Ext.create('Ext.form.Panel', {
			border:false,
			items: [{
				xtype: 'container',
				overflowY: 'scroll',
				layout: {
					align: 'stretch',
					type: 'vbox'
				},
				autoRender: true,
				border: false,
				items: items
			}],
			dockedItems: [{
				xtype: 'container',
				flex: 1,
				dock: 'bottom',
				layout: {
					align: 'middle',
					pack: 'center',
					type: 'hbox'
				},	
				border: false,
				items: [{
					xtype: 'button',
					formBind: true,
					disabled: true,
					text: 'Закрыть',
					minWidth:100,
					margin: 10,
					listeners: {
						click: {
							fn: function(button, e, eOpts)
							{
								var window = button.up('window');
								window.close();
							},	
							scope: this
						}
					}
				},
				{
					xtype: 'button',
					formBind: true,
					disabled: true,
					text: config.buttonText,
					margin: 10,
					minWidth: 100,
					listeners: {
						click: {
							fn: function(button, e, eOpts)
							{
								var window = button.up('window');
								var form = button.up('form');

								if (form.isDirty() == false) {
									config.success(form, null);
									return;
								}

								var mask = Ext.getBody().mask('Сохранение данных', 'Сохранение');
								mask.setStyle('z-index', Ext.WindowMgr.zseed + 1000);

								form.submit({									

									clientValidation: true,
									params: config.params,
									headers: config.headers,

									success: function(form, action) {
										Ext.getBody().unmask();
										config.success(form, action);
									},

									failure: function(form, action) {

										Ext.getBody().unmask();

										switch (action.failureType) {

											case Ext.form.action.Action.CLIENT_INVALID:
											Ext.Msg.alert('Failure', 'Form fields may not be submitted with invalid values');
											break;
											case Ext.form.action.Action.CONNECT_FAILURE:
											Ext.Msg.alert('Failure', 'Ajax communication failed');
											break;
											case Ext.form.action.Action.SERVER_INVALID:
											Ext.Msg.alert('Failure', action.result.message);
											break;
										}
									}
								});
							},	
							scope: this
						}
					}
				}],		
			}],			

			layout: 'fit',
			bodyPadding: 10,
			header: false,	
			url: config.url,
			action: config.action,
			jsonSubmit: true
		});

		processConditions();

		return panel;
	},

	launch: function() {
		if( Ext.supports.LocalStorage )
		{
			Ext.state.Manager.setProvider(new Ext.state.LocalStorageProvider());
		}
		else
		{
			Ext.state.Manager.setProvider(new Ext.state.CookieProvider());
		}

		Ext.define('TC.Ext.picker.Date', {
			extend: 'Ext.picker.Date',
			alias: 'widget.tc_datepicker',
			selectToday: function(val) {
				this.up('window').down('tc_datemenu').hide();
				this.up('window').down('#interval').show();
				return this;
			}
		});


		Ext.define('TC.Ext.menu.DatePicker', {
			extend: 'Ext.menu.Menu',

			alias: 'widget.tc_datemenu',

			requires: [
			'TC.Ext.picker.Date'
			],

			/**
			* @cfg {Boolean} hideOnClick
			* False to continue showing the menu after a date is selected.
			*/
			hideOnClick : true,

			/**
			* @cfg {String} pickerId
			* An id to assign to the underlying date picker.
			*/
			pickerId : null,

			/**
			* @cfg {Number} maxHeight
			* @private
			*/

			/**
			* @property {Ext.picker.Date} picker
			* The {@link Ext.picker.Date} instance for this DateMenu
			*/

			initComponent : function(){
				var me = this,
					cfg = Ext.apply({}, me.initialConfig);

				// Ensure we clear any listeners so they aren't duplicated
				delete cfg.listeners;

				Ext.apply(me, {
					showSeparator: false,
					plain: true,
					border: false,
					bodyPadding: 0, // remove the body padding from the datepicker menu item so it looks like 3.3
					items: Ext.applyIf({
						cls: Ext.baseCSSPrefix + 'menu-date-item',
						id: me.pickerId,
						xtype: 'tc_datepicker'
					}, cfg)
				});

				me.callParent(arguments);

				me.picker = me.down('datepicker');
				/**
				* @event select
				* @inheritdoc Ext.picker.Date#select
				*/
				me.relayEvents(me.picker, ['select']);

				if (me.hideOnClick) {
					me.on('select', me.hidePickerOnSelect, me);
				}
			},

			hidePickerOnSelect: function() {
				Ext.menu.Manager.hideAll();
			}
		});

		Ext.apply(Ext.form.VTypes, {
			daterange : function(val, field) {

				var date = field.parseDate(val);

				if(!date){
					return;
				}

				/*		if (field.startDateField) {

				var start = Ext.getCmp(field.startDateField);

				if (!start.maxValue || (date.getTime() != start.maxValue.getTime())) {
				start.maxValue = date;
				start.setMaxValue(date);
				start.validate();
				}
				} 
				else */if (field.endDateField) {

				var end = Ext.getCmp(field.endDateField);

				if (!end.minValue || (date.getTime() != end.minValue.getTime())) {
					end.minValue = date;
					end.setMinValue(date);
					end.setMaxValue(new Date(date.getTime() + 86400 * 1000 * 7));
					end.validate();
				}
			}
			return true;
		}
	});

	Ext.define('Sandbox.view.SearchTrigger', {
		extend: 'Ext.form.field.Trigger',
		alias: 'widget.searchtrigger',
		triggerCls: 'x-form-clear-trigger',
		trigger2Cls: 'x-form-search-trigger',
		onTriggerClick: function() {
			this.setValue('')
			this.setFilter(this.up().dataIndex, '')
		},
		onTrigger2Click: function() {
			this.setFilter(this.up().dataIndex, this.getValue())
		},
		setFilter: function(filterId, value){
			var store = this.up('grid').getStore();
			if(value){
				store.removeFilter(filterId, false)
				var filter = {id: filterId, property: filterId, value: value};
				if(this.anyMatch) filter.anyMatch = this.anyMatch
				if(this.caseSensitive) filter.caseSensitive = this.caseSensitive
				if(this.exactMatch) filter.exactMatch = this.exactMatch
				if(this.operator) filter.operator = this.operator
				console.log(this, filter)
				store.addFilter(filter)
			} else {
				store.filters.removeAtKey(filterId)
				store.reload()
			}
		},
		listeners: {
			render: function(){
				var me = this;
				me.ownerCt.on('resize', function(){
					me.setWidth(this.getEl().getWidth())
				})
			},
			change: function() {
				if(this.autoSearch) this.setFilter(this.up().dataIndex, this.getValue())
			}
		}
	});

	Ext.define('Sandbox.view.OwnersGrid', {
		extend: 'Ext.grid.Panel',
		requires: ['Sandbox.view.SearchTrigger'],
		alias: 'widget.filterGrid',
		columns: [{
			dataIndex: 'id',
			width: 50,
			text: 'ID'
		}, {
			dataIndex: 'name',
			text: 'Name',
			items:[{
				xtype: 'searchtrigger',
				autoSearch: true
			}]
		}]
	});
	},

	loadTree: function(loadLast, success) {
		var mask = Ext.getBody().mask('Загрузка данных', 'Загрузка');
		mask.setStyle('z-index', Ext.WindowMgr.zseed + 1000);

		Ext.Ajax.request({
			url: '/tree',
			scope:this,
			headers : { Authorization : MyApp.AuthString },

			success: function(response, options){

				Ext.getBody().unmask();

				var root = Ext.decode(response.responseText).object;

				function set_leafs(object)
				{
					if (object.login) {
						object.text = object.login;
					}
					else {
						object.text = object.name;
					}

					if (object.type >= 1000) {

						object.leaf = true;
						object.checked = loadLast;				
					}
					else {
						object.checked = false;
					}

					if (typeof object.children != 'undefined') {
						if (object.children.length !== 0) {

							for (var key in object.children) {
								set_leafs(object.children[key]);
							}
						}
					}

					object.selected = false;

					if (object.custom) {

						try {
							object.custom = Ext.decode(object.custom);
						}
						catch (e) {
							delete object.custom;
						}
					}
				}

				set_leafs(root);

				root.expanded = true;

				if (success) {
					success(root);
				}
			},

			failure: function(response, options){

				Ext.getBody().unmask();

				var statusCode = response.status;
				var statusText = response.statusText;

				if (statusCode == 401) {
					Ext.MessageBox.show({
						title: 'Ошибка',
						msg: 'Введены неправильные имя пользователя и/или пароль.',
						buttons: Ext.MessageBox.OK,
						icon: Ext.MessageBox.ERROR
					});
				}
				else {
					Ext.MessageBox.show({
						title: 'Ошибка',
						msg: statusText + '[' + statusCode + ']',
						buttons: Ext.MessageBox.OK,
						icon: Ext.MessageBox.ERROR
					});
				}
			},
			timeout: 60000
		});
	}

});