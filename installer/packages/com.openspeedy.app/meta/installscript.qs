function Component()
{
}

Component.prototype.createOperations = function()
{
    try 
    {
        // call the base create operations function
        component.createOperations();
        
        component.addOperation("CreateShortcut", "@TargetDir@/OpenSpeedy.exe", "@DesktopDir@/OpenSpeedy.lnk");
    } 
    catch (e) 
    {
        console.log(e);
    }
}
