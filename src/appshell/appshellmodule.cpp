/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "appshellmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/applicationuiactions.h"
#include "internal/applicationactioncontroller.h"
#include "internal/appshellconfiguration.h"
#include "internal/startupscenario.h"
#include "internal/applicationactioncontroller.h"
#include "internal/sessionsmanager.h"

#include "view/devtools/settingslistmodel.h"
#include "view/mainwindowtitleprovider.h"
#include "view/notationpagemodel.h"
#include "view/notationstatusbarmodel.h"
#include "view/aboutmodel.h"
#include "view/welcomedialogmodel.h"
#include "view/firstlaunchsetup/firstlaunchsetupmodel.h"
#include "view/firstlaunchsetup/themespagemodel.h"
#include "view/firstlaunchsetup/tutorialspagemodel.h"
#include "view/preferences/preferencesmodel.h"
#include "view/preferences/generalpreferencesmodel.h"
#include "view/preferences/updatepreferencesmodel.h"
#include "view/preferences/appearancepreferencesmodel.h"
#include "view/preferences/folderspreferencesmodel.h"
#include "view/preferences/noteinputpreferencesmodel.h"
#include "view/preferences/advancedpreferencesmodel.h"
#include "view/preferences/canvaspreferencesmodel.h"
#include "view/preferences/saveandpublishpreferencesmodel.h"
#include "view/preferences/scorepreferencesmodel.h"
#include "view/preferences/importpreferencesmodel.h"
#include "view/preferences/audiomidipreferencesmodel.h"
#include "view/preferences/percussionpreferencesmodel.h"
#include "view/preferences/commonaudioapiconfigurationmodel.h"
#include "view/preferences/braillepreferencesmodel.h"
#include "view/publish/publishtoolbarmodel.h"
#include "view/internal/maintoolbarmodel.h"

#ifdef Q_OS_MAC
#include "view/appmenumodel.h"
#include "view/internal/platform/macos/macosappmenumodelhook.h"
#else
#include "view/navigableappmenumodel.h"
#endif

using namespace mu::appshell;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::dock;

static void appshell_init_qrc()
{
    Q_INIT_RESOURCE(appshell);
}

std::string AppShellModule::moduleName() const
{
    return "appshell";
}

void AppShellModule::registerExports()
{
    m_applicationActionController = std::make_shared<ApplicationActionController>(iocContext());
    m_applicationUiActions = std::make_shared<ApplicationUiActions>(m_applicationActionController, iocContext());
    m_appShellConfiguration = std::make_shared<AppShellConfiguration>(iocContext());
    m_sessionsManager = std::make_shared<SessionsManager>(iocContext());

    ioc()->registerExport<IAppShellConfiguration>(moduleName(), m_appShellConfiguration);
    ioc()->registerExport<IStartupScenario>(moduleName(), new StartupScenario(iocContext()));
    ioc()->registerExport<ISessionsManager>(moduleName(), m_sessionsManager);

#ifdef Q_OS_MAC
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<MacOSAppMenuModelHook>());
#else
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<AppMenuModelHookStub>());
#endif
}

void AppShellModule::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_applicationUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("notateai://home"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("notateai://notation"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("notateai://sequencer"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("notateai://publish"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("notateai://devtools"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("notateai://about/notateai"), ContainerMeta(ContainerType::QmlDialog, "AboutDialog.qml"));
        ir->registerUri(Uri("notateai://about/musicxml"), ContainerMeta(ContainerType::QmlDialog, "AboutMusicXMLDialog.qml"));
        ir->registerUri(Uri("notateai://welcomedialog"), ContainerMeta(ContainerType::QmlDialog, "WelcomeDialog.qml"));
        ir->registerUri(Uri("notateai://firstLaunchSetup"),
                        ContainerMeta(ContainerType::QmlDialog, "FirstLaunchSetup/FirstLaunchSetupDialog.qml"));
        ir->registerUri(Uri("muse://preferences"), ContainerMeta(ContainerType::QmlDialog, "Preferences/PreferencesDialog.qml"));
    }
}

void AppShellModule::registerResources()
{
    appshell_init_qrc();
}

void AppShellModule::registerUiTypes()
{
    qmlRegisterType<SettingListModel>("NotateAI.Preferences", 1, 0, "SettingListModel");
    qmlRegisterType<PreferencesModel>("NotateAI.Preferences", 1, 0, "PreferencesModel");
    qmlRegisterType<GeneralPreferencesModel>("NotateAI.Preferences", 1, 0, "GeneralPreferencesModel");
    qmlRegisterType<UpdatePreferencesModel>("NotateAI.Preferences", 1, 0, "UpdatePreferencesModel");
    qmlRegisterType<AppearancePreferencesModel>("NotateAI.Preferences", 1, 0, "AppearancePreferencesModel");
    qmlRegisterType<FoldersPreferencesModel>("NotateAI.Preferences", 1, 0, "FoldersPreferencesModel");
    qmlRegisterType<NoteInputPreferencesModel>("NotateAI.Preferences", 1, 0, "NoteInputPreferencesModel");
    qmlRegisterType<AdvancedPreferencesModel>("NotateAI.Preferences", 1, 0, "AdvancedPreferencesModel");
    qmlRegisterType<CanvasPreferencesModel>("NotateAI.Preferences", 1, 0, "CanvasPreferencesModel");
    qmlRegisterType<SaveAndPublishPreferencesModel>("NotateAI.Preferences", 1, 0, "SaveAndPublishPreferencesModel");
    qmlRegisterType<ScorePreferencesModel>("NotateAI.Preferences", 1, 0, "ScorePreferencesModel");
    qmlRegisterType<ImportPreferencesModel>("NotateAI.Preferences", 1, 0, "ImportPreferencesModel");
    qmlRegisterType<AudioMidiPreferencesModel>("NotateAI.Preferences", 1, 0, "AudioMidiPreferencesModel");
    qmlRegisterType<PercussionPreferencesModel>("NotateAI.Preferences", 1, 0, "PercussionPreferencesModel");
    qmlRegisterType<CommonAudioApiConfigurationModel>("NotateAI.Preferences", 1, 0, "CommonAudioApiConfigurationModel");
    qmlRegisterType<BraillePreferencesModel>("NotateAI.Preferences", 1, 0, "BraillePreferencesModel");

#if defined(Q_OS_MACOS)
    qmlRegisterType<AppMenuModel>("NotateAI.AppShell", 1, 0, "PlatformAppMenuModel");
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    qmlRegisterType<AppMenuModel>("NotateAI.AppShell", 1, 0, "PlatformAppMenuModel");
    qmlRegisterType<NavigableAppMenuModel>("NotateAI.AppShell", 1, 0, "AppMenuModel");
#else
    qmlRegisterType<NavigableAppMenuModel>("NotateAI.AppShell", 1, 0, "AppMenuModel");
#endif

    qmlRegisterType<MainWindowTitleProvider>("NotateAI.AppShell", 1, 0, "MainWindowTitleProvider");
    qmlRegisterType<NotationPageModel>("NotateAI.AppShell", 1, 0, "NotationPageModel");
    qmlRegisterType<NotationStatusBarModel>("NotateAI.AppShell", 1, 0, "NotationStatusBarModel");
    qmlRegisterType<AboutModel>("NotateAI.AppShell", 1, 0, "AboutModel");
    qmlRegisterType<WelcomeDialogModel>("NotateAI.AppShell", 1, 0, "WelcomeDialogModel");
    qmlRegisterType<FirstLaunchSetupModel>("NotateAI.AppShell", 1, 0, "FirstLaunchSetupModel");
    qmlRegisterType<ThemesPageModel>("NotateAI.AppShell", 1, 0, "ThemesPageModel");
    qmlRegisterType<TutorialsPageModel>("NotateAI.AppShell", 1, 0, "TutorialsPageModel");
    qmlRegisterType<PublishToolBarModel>("NotateAI.AppShell", 1, 0, "PublishToolBarModel");
    qmlRegisterType<MainToolBarModel>("NotateAI.AppShell", 1, 0, "MainToolBarModel");
}

void AppShellModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_applicationActionController->preInit();
}

void AppShellModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_appShellConfiguration->init();
    m_applicationActionController->init();
    m_applicationUiActions->init();
    m_sessionsManager->init();
}

void AppShellModule::onAllInited(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    //! NOTE: process QEvent::FileOpen as early as possible if it was postponed
#ifdef Q_OS_MACOS
    qApp->processEvents();
#endif
}

void AppShellModule::onDeinit()
{
    m_sessionsManager->deinit();
}
