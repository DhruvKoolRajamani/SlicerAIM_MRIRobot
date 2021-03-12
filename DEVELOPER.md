# Developer Instructions

## Add a workspace generation algorithm

1. Add a button to the [UI](WorkspaceGeneration/Resources/UI/qSlicerWorkspaceGenerationModuleWidget.ui)
2. Rebuild the project
3. Navigate to [qSlicerWorkspaceGenerationModuleWidget::setup():L131](WorkspaceGeneration/qSlicerWorkspaceGenerationModuleWidget.cxx#L131)
4. Connect the button to an event handler
5. Copy and paste the logic inside and then edit [qSlicerWorkspaceGenerationModuleWidget::onGenerateWorkspaceClick()](WorkspaceGeneration/qSlicerWorkspaceGenerationModuleWidget.cxx)
6. ...
