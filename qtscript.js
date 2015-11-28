function Controller() 
{ 
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, true); 
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false); 
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false); 
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, true); 
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, true); 
} 
Controller.prototype.TargetDirectoryPageCallback = function() 
{ 
    var targetDir = installer.environmentVariable("TARGET_DIR"); 
	var page = gui.pageWidgetByObjectName("TargetDirectoryPage"); 
	page.TargetDirectoryLineEdit.setText(targetDir); 
	gui.clickButton(buttons.NextButton); 
} 
Controller.prototype.StartMenuDirectoryPageCallback = function() 
{ 
	gui.clickButton(buttons.NextButton); 
} 
