/*
 * File: app/view/GroupWindow.js
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

Ext.define('MyApp.view.GroupWindow', {
	extend: 'Ext.window.Window',

	id: 'userCreate1',
	layout: {
		type: 'fit'
	},
	title: 'Группа',

	initComponent: function() {
		var me = this;

		Ext.applyIf(me, {
			items: [
				{
					xtype: 'form',
					border: false,
					id: 'formPanel1',
					bodyPadding: 10,
					header: false,
					title: 'My Form',
					jsonSubmit: true,
					trackResetOnLoad: true,
					items: [
						{
							xtype: 'textfield',
							anchor: '100%',
							minWidth: 400,
							fieldLabel: 'Название',
							labelWidth: 150,
							name: 'name',
							maxLength: 256,
							maxLengthText: 'Название не должно содержать более {0} символов'
						}
					],
					dockedItems: [
						{
							xtype: 'container',
							dock: 'bottom',
							layout: {
								align: 'middle',
								pack: 'center',
								type: 'hbox'
							},
							items: [
								{
									xtype: 'button',
									minWidth: 100,
									text: 'Отмена',
									listeners: {
										click: {
											fn: me.onButtonClick1,
											scope: me
										}
									}
								},
								{
									xtype: 'button',
									formBind: true,
									margin: 10,
									minWidth: 100,
									text: 'ОК',
									listeners: {
										click: {
											fn: me.onButtonClick,
											scope: me
										}
									}
								}
							]
						}
					]
				}
			],
			listeners: {
				beforeshow: {
					fn: me.onWindowBeforeShow,
					scope: me
				}
			}
		});

		me.callParent(arguments);
	},

	onButtonClick1: function(button, e, eOpts) {
		button.up('window').close();
	},

	onButtonClick: function(button, e, eOpts) {
		var form = button.up('form').getForm();
		var window = button.up('window');

		if ((form.method != 'PUT')&&(!form.isDirty())) {
			window.close();
			return;
		}

		var mask = Ext.getBody().mask('Сохранение данных', 'Сохранение');
		mask.setStyle('z-index', Ext.WindowMgr.zseed + 1000);


		form.submit({									

			clientValidation: true,
			headers : { Authorization : MyApp.AuthString },

			success: function(form, action) {

				Ext.getBody().unmask();

				MyApp.reload = true;

				window.close();
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

	onWindowBeforeShow: function(component, eOpts) {
		var window = component;
		var form = window.down('form').getForm();

		if (typeof window.group_id != 'undefined') {

			form.method = 'POST';
			form.url = '/objects/' + window.group_id;
			window.setTitle('Изменение настроек группы');

			var mask = Ext.getBody().mask('Получение данных', 'Загрузка');
			mask.setStyle('z-index', Ext.WindowMgr.zseed + 1000);

			Ext.Ajax.request({
				url: form.url,
				scope: this,
				headers : { Authorization : MyApp.AuthString },

				success: function(response, options){

					Ext.getBody().unmask();

					data = Ext.decode(response.responseText).object;
					form.setValues(data);
				},

				failure: function(response, options){

					Ext.getBody().unmask();

					var statusCode = response.status;
					var statusText = response.statusText;

					Ext.MessageBox.show({
						title: 'Ошибка',
						msg: statusText + '[' + statusCode + ']',
						buttons: Ext.MessageBox.OK,
						icon: Ext.MessageBox.ERROR
					});

					window.close();
				},

				timeout: 60000
			});				
		}
		else {	
			form.method = 'PUT';
			form.url = '/objects/' + window.parent_id + '/groups';
			window.setTitle('Добавление группы');
		}									


	}

});