<?php
/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (C) 2009-2014 Volker Theile
 * @copyright Copyright (C) 2014 OpenMediaVault Plugin Developers
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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/initscript.inc");

class OMVModuleSyncthing extends OMVModuleServiceAbstract implements
    OMVINotifyListener,
    OMVIModuleServiceStatus
{

    public function getName()
    {
        return "Syncthing";
    }

    private function getPluginName()
    {
        return strtolower( $this->getName() );
    }

    private function getXPath()
    {
        return sprintf( "//services/%s", $this->getPluginName() );
    }

    private function getXPathUsers()
    {
        return sprintf( "%s/users", $this->getXPath() );
    }

    private function getXPathUser()
    {
        return sprintf( "%s/user", $this->getXPathUsers() );
    }

    private function getEventMessagePath()
    {
        return sprintf( "org.openmediavault.services.%s", $this->getPluginName() );
    }

    private function getEventMessagePathSettings()
    {
        return sprintf( "%s.settings", $this->getEventMessagePath());
    }

    private function getEventMessagePathUser()
    {
        return sprintf( "%s.users.user", $this->getEventMessagePath());
    }

    public function getStatus()
    {
        global $xmlConfig;

        $xPath = sprintf("%s[enable='1']", $this->getXPathUser());
        $initd = new OMVSysVInitScript( $this->getPluginName() );
        return array(
            "name"    => $this->getName(),
            "title"   => gettext( $this->getName() ),
            "enabled" => $xmlConfig->exists($xPath),
            "running" => ($initd->status() == 0)
        );
    }

    public function applyConfig()
    {
        $cmd = sprintf("omv-mkconf %s 2>&1", $this->getPluginName());

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }
    }

    public function startService()
    {
        global $xmlConfig;

        $xPath = sprintf("%s[enable='1']", $this->getXPathUser());
        if ( TRUE === $xmlConfig->exists($xPath) ) {
            $initd = new OMVSysVInitScript($this->getPluginName(), true);
            $initd->exec();
        }

        $xPath = sprintf("%s[enable='1']", $this->getXPathSettings());
        if ( TRUE === $xmlConfig->exists($xPath) ) {
            $initd = new OMVSysVInitScript("discosrv", true);
            $initd->exec();
        }
    }

    public function stopService()
    {
        $initd = new OMVSysVInitScript($this->getPluginName(), false);
        $initd->exec();

        $initd = new OMVSysVInitScript("discosrv", false);
        $initd->exec();
    }

    public function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathSettings(),
            array($this, "setDirty")
        );

        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathUser(),
            array($this, "setDirty")
        );
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleSyncthing());
