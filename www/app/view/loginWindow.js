/*
 * File: app/view/loginWindow.js
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

Ext.define('MyApp.view.loginWindow', {
	extend: 'Ext.window.Window',

	hidden: false,
	layout: {
		type: 'fit'
	},
	closable: false,
	title: 'Авторизация',
	titleAlign: 'center',
	modal: true,

	initComponent: function() {
		var me = this;

		Ext.applyIf(me, {
			items: [
				{
					xtype: 'form',
					bodyPadding: 10,
					header: false,
					title: 'My Form',
					items: [
						{
							xtype: 'textfield',
							anchor: '100%',
							id: 'loginEdit',
							fieldLabel: 'Имя пользователя',
							labelWidth: 150,
							name: 'login',
							allowBlank: false,
							allowOnlyWhitespace: false,
							blankText: 'Необходимо указть имя пользователя',
							emptyText: 'Имя пользователя',
							listeners: {
								specialkey: {
									fn: me.onLoginEditSpecialkey,
									scope: me
								}
							}
						},
						{
							xtype: 'textfield',
							anchor: '100%',
							id: 'passwordEdit',
							fieldLabel: 'Пароль:',
							labelWidth: 150,
							name: 'password',
							inputType: 'password',
							allowBlank: false,
							allowOnlyWhitespace: false,
							blankText: 'Необходимо ввести пароль',
							emptyText: 'Пароль',
							listeners: {
								specialkey: {
									fn: me.onPasswordEditSpecialkey,
									scope: me
								}
							}
						},
						{
							xtype: 'checkboxfield',
							anchor: '100%',
							itemId: 'loadlast',
							name: 'loadlast',
							boxLabel: 'Загрузить последние координаты',
							checked: true
						},
						{
							xtype: 'checkboxfield',
							anchor: '100%',
							fieldLabel: 'Label',
							hideLabel: true,
							name: 'remember',
							boxLabel: 'Запомнить меня',
							checked: true
						}
					]
				}
			],
			dockedItems: [
				{
					xtype: 'toolbar',
					dock: 'bottom',
					items: [
						{
							xtype: 'button',
							text: 'Вход',
							listeners: {
								click: {
									fn: me.onButtonClick,
									scope: me
								}
							}
						},
						{
							xtype: 'button',
							text: 'Забыл пароль'
						},
						{
							xtype: 'button',
							text: 'Регистрация'
						}
					]
				}
			],
			listeners: {
				afterrender: {
					fn: me.onWindowAfterRender,
					scope: me
				}
			}
		});

		me.callParent(arguments);
	},

	onLoginEditSpecialkey: function(field, e, eOpts) {
		if (e.getKey() == e.ENTER) {
			var button = field.up('window').down('button');
			button.fireEvent('click', button);
		}
	},

	onPasswordEditSpecialkey: function(field, e, eOpts) {
		if (e.getKey() == e.ENTER) {
			var button = field.up('window').down('button');
			button.fireEvent('click', button);
		}
	},

	onButtonClick: function(button, e, eOpts) {
		var form = button.up('window').down('form');

		if (form.isValid())
		{
			form.updateRecord();

			var record = form.getRecord();

			MyApp.AuthString = 'Basic ' + Base64.encode(record.get('login') + ':' + record.get('password'));

			if (record.get('remember') !== true) {
				record.set('password', '');
			}

			MyApp.localStorage.setItem('auth', 'true');
			MyApp.localStorage.setItem('login', record.get('login'));
			MyApp.localStorage.setItem('password', record.get('password'));
			MyApp.localStorage.setItem('loadlast', record.get('loadlast') ? 'true' : 'false');
			MyApp.localStorage.setItem('remember', record.get('remember') ? 'true' : 'false');

			MyApp.getApplication().loadTree(record.get('loadlast'), 
			function(tree) {
				button.up('window').close();
				MyApp.tree = tree;
				Ext.create('MyApp.view.MainWindow').show();
			}
			);
		}

	},

	onWindowAfterRender: function(component, eOpts) {
		var localStorage = window.localStorage;

		MyApp.localStorage = localStorage;

		console.log(localStorage);

		var record;

		if (localStorage.getItem('auth') == 'true') {
			record = Ext.create('MyApp.model.Auth', {
				login: localStorage.getItem('login'), 
				password: localStorage.getItem('password'), 
				remember: (localStorage.getItem('remember') == 'true') ? true : false,
				loadlast: (localStorage.getItem('loadlast') == 'true') ? true : false
			});
		}
		else {
			record = Ext.create('MyApp.model.Auth', {
				login: 'demo', 
				password: 'demo', 
				remember: true,
				loadlast: true
			});
		}

		component.down('form').loadRecord(record);
	}

});