/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtGui>
#include <QDebug>
#include <QButtonGroup>
#include <QFileDialog>

#include "qSlicerApplication.h"

// SlicerQt includes
#include "qSlicerWorkspaceGenerationModuleWidget.h"
#include "ui_qSlicerWorkspaceGenerationModuleWidget.h"

// Slicer includes
#include "vtkSmartPointer.h"
#include "vtkProperty.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkXMLImageDataReader.h"
#include "vtkImageData.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLInteractionNode.h"

// module includes
#include "vtkMRMLWorkspaceGenerationNode.h"
#include "vtkSlicerWorkspaceGenerationLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerWorkspaceGenerationModuleWidgetPrivate : public Ui_qSlicerWorkspaceGenerationModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceGenerationModuleWidget);

protected:
  qSlicerWorkspaceGenerationModuleWidget *const q_ptr;

public:
  qSlicerWorkspaceGenerationModuleWidgetPrivate(qSlicerWorkspaceGenerationModuleWidget &object);
  vtkSlicerWorkspaceGenerationLogic *logic() const;

  // Observed nodes (to keep GUI up-to-date)
  vtkWeakPointer<vtkMRMLWorkspaceGenerationNode> WorkspaceGenerationNode;
  vtkWeakPointer<vtkMRMLMarkupsDisplayNode> MarkupsDisplayNode;
  vtkWeakPointer<vtkMRMLModelDisplayNode> ModelDisplayNode;
};

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidgetPrivate::qSlicerWorkspaceGenerationModuleWidgetPrivate(qSlicerWorkspaceGenerationModuleWidget &object) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkSlicerWorkspaceGenerationLogic *qSlicerWorkspaceGenerationModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerWorkspaceGenerationModuleWidget);
  return vtkSlicerWorkspaceGenerationLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidget::qSlicerWorkspaceGenerationModuleWidget(QWidget *_parent)
    : Superclass(_parent), d_ptr(new qSlicerWorkspaceGenerationModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidget::~qSlicerWorkspaceGenerationModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::setup()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Connect buttons in UI
  this->setMRMLScene(d->logic()->GetMRMLScene());

  vtkMRMLWorkspaceGenerationNode *selectedWorkspaceGenerationNode = vtkMRMLWorkspaceGenerationNode::New();
  qvtkReconnect(d->WorkspaceGenerationNode, selectedWorkspaceGenerationNode, vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
  d->WorkspaceGenerationNode = selectedWorkspaceGenerationNode;
  d->WorkspaceGenerationNode->DebugOn();
  
  connect(d->InputDataSelector_1, SIGNAL(currentNodeChanged(vtkMRMLNode *)), this, SLOT(onInputDataSelectionChanged()));
  connect(d->WorkspaceOFDBtn_1, SIGNAL(released()), this, SLOT(onWorkspaceOFDButtonClick()));
  connect(d->WorkspaceLoadBtn_1, SIGNAL(released()), this, SLOT(onWorkspaceLoadButtonClick()));
  connect(d->SaveSceneBtn_1, SIGNAL(released()), this, SLOT(onSaveSceneButtonClick()));
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::setMRMLScene(vtkMRMLScene *scene)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  this->Superclass::setMRMLScene(scene);
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputDataSelectionChanged()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  vtkMRMLVolumeNode* selectedInputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->InputDataSelector_1->currentNode());
  qvtkReconnect(inputVolumeNode, selectedInputVolumeNode, vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
  if (inputVolumeNode != NULL)
  {
    d->logic()->UpdateSelectionNode(d->WorkspaceGenerationNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceOFDButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  auto fileName = QFileDialog::getOpenFileName(this, tr("Open Workspace Mesh"), QDir::currentPath(), tr("Polymesh File (*.ply)"));
  d->WorkspacePathInputLineEdit_1->setText(fileName);
  // vtkMRMLWorkspaceGenerationNode *selectedWorkspaceGenerationNode = vtkMRMLWorkspaceGenerationNode::SafeDownCast(d->InputDataSelector_1->currentNode());
  // qvtkReconnect(d->WorkspaceGenerationNode, selectedWorkspaceGenerationNode, vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
  // d->WorkspaceGenerationNode = selectedWorkspaceGenerationNode;
  // d->logic()->UpdateSelectionNode(d->WorkspaceGenerationNode);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceLoadButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  // auto fileName = QFileDialog::getOpenFileName(this, tr("Open Workspace Mesh"), QDir::currentPath(), tr("Polymesh File (*.ply)"));
  // d->WorkspacePathInputLineEdit_1->setText(fileName);
  // // vtkMRMLWorkspaceGenerationNode *selectedWorkspaceGenerationNode = vtkMRMLWorkspaceGenerationNode::SafeDownCast(d->InputDataSelector_1->currentNode());
  // qvtkReconnect(d->WorkspaceGenerationNode, selectedWorkspaceGenerationNode, vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
  // d->WorkspaceGenerationNode = selectedWorkspaceGenerationNode;
  // d->logic()->UpdateSelectionNode(d->WorkspaceGenerationNode);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onSaveSceneButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  if (inputVolumeNode != NULL)
  {
    vtkSmartPointer<vtkXMLImageDataWriter> writer =
        vtkSmartPointer<vtkXMLImageDataWriter>::New();
    vtkSmartPointer<vtkImageData> imageData = inputVolumeNode->GetImageData();
    writer->SetInputData(imageData);
    writer->SetFileName("/home/dhruv/testvolume.vti");
    writer->Write();
    writer->Delete();
  }
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onSceneImportedEvent()
{
  // Replace with registration/generation logic?
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enter()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  this->Superclass::enter();

  if (this->mrmlScene() == NULL)
  {
    qCritical() << "Invalid scene!";
    return;
  }

  d->InputDataSelector_1->addEnabled();
  d->SaveSceneBtn_1->setDisabled(true);

  QStringList qNodeTypes = {"vtkMRMLVolumeNode"};
  d->InputDataSelector_1->setNodeTypes(qNodeTypes);

  // For convenience, select a default parameter node.
  if (d->InputDataSelector_1->currentNode() == NULL)
  {
    d->InputDataSelector_1->setMRMLScene(this->mrmlScene());
  }

  // Need to update the GUI so that it observes whichever parameter node is selected
  this->onInputDataSelectionChanged();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::exit()
{
  Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::updateGUIFromMRML()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->InputDataSelector_1->currentNode());
  if (inputVolumeNode == NULL)
  {
    qCritical("Selected node not a valid module node");
    this->enableAllWidgets(false);
    return;
  }

  if (!d->SaveSceneBtn_1->isEnabled())
    d->SaveSceneBtn_1->setEnabled(true);

  this->enableAllWidgets(true); // unless otherwise specified, everything is enabled

  // Node selectors
  // vtkMRMLNode *inputNode = inputVolumeNode->GetInputNode();
  // d->InputDataSelector_1->setCurrentNode(inputNode);

  // vtkMRMLMarkupsFiducialNode *inputMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(inputNode);
  // if (inputMarkupsFiducialNode != NULL)
  // {
  //   d->InputMarkupsPlaceWidget->setCurrentNode(inputMarkupsFiducialNode);
  // }
  // else
  // {
  //   d->InputMarkupsPlaceWidget->setCurrentNode(NULL); // not a markups node
  // }
  // d->ModelNodeSelector->setCurrentNode(workspaceGenerationModuleNode->GetOutputModelNode());

  // block ALL signals until the function returns
  // if a return is called after this line, then unblockAllSignals should also be called.
  this->blockAllSignals(true);

  // Model display options
  // vtkMRMLModelDisplayNode *modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
  //     this->GetOutputModelNode() ? this->GetOutputModelNode()->GetDisplayNode() : NULL);
  // if (modelDisplayNode != NULL)
  // {
  //   d->ModelVisiblityButton->setChecked(modelDisplayNode->GetVisibility());
  //   d->ModelOpacitySlider->setValue(modelDisplayNode->GetOpacity());
  //   double *outputColor = modelDisplayNode->GetColor();
  //   QColor nodeOutputColor;
  //   nodeOutputColor.setRgbF(outputColor[0], outputColor[1], outputColor[2]);
  //   d->ModelColorSelector->setColor(nodeOutputColor);
  //   d->ModelSliceIntersectionCheckbox->setChecked(modelDisplayNode->GetSliceIntersectionVisibility());
  // }
  // else
  // {
  //   d->ModelVisiblityButton->setChecked(false);
  //   d->ModelOpacitySlider->setValue(1.0);
  //   QColor nodeOutputColor;
  //   nodeOutputColor.setRgbF(0, 0, 0);
  //   d->ModelColorSelector->setColor(nodeOutputColor);
  //   d->ModelSliceIntersectionCheckbox->setChecked(false);
  // }
  // d->ModelVisiblityButton->setEnabled(modelDisplayNode != NULL);
  // d->ModelOpacitySlider->setEnabled(modelDisplayNode != NULL);
  // d->ModelColorSelector->setEnabled(modelDisplayNode != NULL);
  // d->ModelSliceIntersectionCheckbox->setEnabled(modelDisplayNode != NULL);

  // // Markups display options
  // vtkMRMLMarkupsFiducialNode *inputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetInputNode());
  // if (inputMarkupsNode != NULL)
  // {
  //   vtkMRMLMarkupsDisplayNode *inputMarkupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(inputMarkupsNode->GetDisplayNode());
  //   if (inputMarkupsDisplayNode != NULL)
  //   {
  //     d->MarkupsTextScaleSlider->setValue(inputMarkupsDisplayNode->GetTextScale());
  //     d->MarkupsTextScaleSlider->setEnabled(true);
  //   }
  //   else
  //   {
  //     d->MarkupsTextScaleSlider->setValue(0);
  //     d->MarkupsTextScaleSlider->setEnabled(false);
  //   }
  // }
  // else
  // {
  //   d->MarkupsTextScaleSlider->setEnabled(false);
  // }

  // // Determine visibility of widgets
  // bool isInputMarkups = (vtkMRMLMarkupsFiducialNode::SafeDownCast(inputNode) != NULL);

  // d->InputMarkupsPlaceWidget->setVisible(isInputMarkups);
  // d->MarkupsTextScaleSlider->setVisible(isInputMarkups);

  // bool isClosedSurface = d->ModeClosedSurfaceRadioButton->isChecked();

  // d->ClosedSurfaceModelGroupBox->setVisible(isClosedSurface);

  // bool isCurve = d->ModeCurveRadioButton->isChecked();

  // d->CurveModelGroupBox->setVisible(isCurve);

  // bool isLinearSpline = d->LinearInterpolationRadioButton->isChecked();
  // bool isCardinalSpline = d->CardinalInterpolationRadioButton->isChecked();
  // bool isKochanekSpline = d->KochanekInterpolationRadioButton->isChecked();
  // bool isSpline = isLinearSpline || isCardinalSpline || isKochanekSpline;

  // d->TubeLoopCheckBox->setEnabled(isSpline);

  // bool isGlobalLeastSquaresPolynomial = d->GlobalLeastSquaresPolynomialApproximationRadioButton->isChecked();
  // bool isMovingLeastSquaresPolynomial = d->MovingLeastSquaresPolynomialApproximationRadioButton->isChecked();
  // bool isPolynomial = isGlobalLeastSquaresPolynomial || isMovingLeastSquaresPolynomial;

  // d->FittingGroupBox->setVisible(isKochanekSpline || isPolynomial);

  // d->KochanekEndsCopyNearestDerivativesLabel->setVisible(isKochanekSpline);
  // d->KochanekEndsCopyNearestDerivativesCheckBox->setVisible(isKochanekSpline);
  // d->KochanekBiasLabel->setVisible(isKochanekSpline);
  // d->KochanekBiasDoubleSpinBox->setVisible(isKochanekSpline);
  // d->KochanekTensionLabel->setVisible(isKochanekSpline);
  // d->KochanekTensionDoubleSpinBox->setVisible(isKochanekSpline);
  // d->KochanekContinuityLabel->setVisible(isKochanekSpline);
  // d->KochanekContinuityDoubleSpinBox->setVisible(isKochanekSpline);

  // d->PointSortingLabel->setVisible(isPolynomial);
  // d->PointSortingFrame->setVisible(isPolynomial);
  // d->PolynomialOrderLabel->setVisible(isPolynomial);
  // d->PolynomialOrderSpinBox->setVisible(isPolynomial);
  // d->PolynomialSampleWidthLabel->setVisible(isMovingLeastSquaresPolynomial);
  // d->PolynomialSampleWidthDoubleSpinBox->setVisible(isMovingLeastSquaresPolynomial);
  // d->WeightFunctionLabel->setVisible(isMovingLeastSquaresPolynomial);
  // d->WeightFunctionFrame->setVisible(isMovingLeastSquaresPolynomial);

  this->blockAllSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::blockAllSignals(bool block)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  d->InputDataSelector_1->blockSignals(block);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enableAllWidgets(bool enable)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  d->InputDataSelector_1->setEnabled(enable);
  d->SaveSceneBtn_1->setEnabled(enable);
}
