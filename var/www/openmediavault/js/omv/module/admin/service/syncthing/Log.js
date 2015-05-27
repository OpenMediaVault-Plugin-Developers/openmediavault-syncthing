/**
 * Copyright (C) 2014-2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/WorkspaceManager.js")
// require("js/omv/window/Window.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/data/Store.js")

Ext.define("OMV.module.admin.service.syncthing.Log", {
    extend: "OMV.window.Window",
    requires: [
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.data.Store"
    ],

    title: _("Log"),

    width: 550,
    height: 400,
    layout: "fit",

    initComponent: function() {
        var grid = this.getGrid();
        var store = grid.getStore();

        Ext.apply(this, {
            items: [grid]
        });

        //store.reload();
        this.callParent(arguments);
    },

    getGrid: function() {
        return Ext.create("Ext.grid.Panel", {
            columns: [{
                header: _("Line"),
                width: 50,
                sortable: true,
                dataIndex: "id"
            }, {
                header: _("Message"),
                flex: 1,
                sortable: false,
                dataIndex: "message"
            }],
            store: Ext.create("OMV.data.Store", {
                autoLoad: true,
                model: OMV.data.Model.createImplicit({
                    idProperty: "id",
                    fields: [{
                        name: "id"
                    }, {
                        name: "message"
                    }]
                }),
                proxy: {
                    type: "rpc",
                    rpcData: {
                        "service": "Syncthing",
                        "method": "getLogView"
                    },
                    extraParams: {
                        username: this.username
                    }
                },
                remoteSort: false,
                sorters: [{
                    direction: "DESC",
                    property: "id"
                }]
            }),
            listeners: {
                cellclick: function(grid, td, cellIndex, record) {
                    Ext.MessageBox.show({
                        title: Ext.String.format(
                            _("Message - row {0}"),
                            record.get("id")
                        ),
                        msg: record.get("message")
                    });
                }
            }
        });
    }
});
