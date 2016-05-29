<?php
/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (C) 2009-2014 Volker Theile
 * @copyright Copyright (C) 2014-2016 OpenMediaVault Plugin Developers
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

class OMVModuleSyncthing extends \OMV\Engine\Module\ServiceAbstract
    implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus
{

    public function getName()
    {
        return "Syncthing";
    }


    public function getStatus()
    {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.syncthing.user");
        $systemCtl = new \OMV\System\SystemCtl("syncthing");
        return array(
            "name" => $this->getName(),
            "title" => gettext("Syncthing"),
            "enabled" => $object->get("enable"),
            "running" => $systemCtl->isActive()
        );
    }

    public function applyConfig()
    {
        $cmd = new \OMV\System\Process("omv-mkconf", "syncthing");
        $cmd->setRedirect2to1(TRUE);
        $cmd->execute();
    }

    public function startService()
    {
        $db = \OMV\Config\Database::getInstance();
        if ( $db->exists("conf.service.syncthing.user",["operator" => "stringEquals","arg0" => "enable" , arg1 => "1" ] ) ) {
            $systemCtl = new \OMV\System\SystemCtl("syncthing");
            $systemCtl->enable(TRUE);
        }
    }

    public function stopService()
    {
        $systemCtl = new \OMV\System\SystemCtl("syncthing");
        $systemCtl->disable(TRUE);
    }

    public function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.services.syncthing.user",
            [ $this, "setDirty" ]
        );
    }
}
