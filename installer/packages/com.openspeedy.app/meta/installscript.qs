function Component()
{
}

Component.prototype.createOperations = function()
{
    try 
    {
        // call the base create operations function
        component.createOperations();
        
        component.addOperation("CreateShortcut", "@TargetDir@/Speedy.exe", "@DesktopDir@/Speedy.lnk");
    } 
    catch (e) 
    {
        console.log(e);
    }
}
