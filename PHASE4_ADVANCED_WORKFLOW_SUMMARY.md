# Phase 4: Advanced Workflow Features - Implementation Summary

## Overview

Phase 4 focuses on enhancing the user workflow with advanced features for dataset organization and management. This phase implements drag-and-drop functionality, batch operations, keyboard shortcuts, and improved subset management.

## Objectives

✅ Implement drag-and-drop samples between subsets  
✅ Add batch selection and batch move to subset  
✅ Implement move sample from subset back to root  
✅ Add keyboard shortcuts (Ctrl+I, Ctrl+E)  
⏭️ Visual subset distribution (deferred to Phase 5)  
⏭️ Automatic split ratios (deferred to Phase 5)  

## Implementation Details

### 1. Drag-and-Drop System

**New Component: `DatasetTreeModel`**
- Custom `QStandardItemModel` subclass with drag-drop support
- Location: `src/gui/DatasetTreeModel.{h,cpp}`
- Implements `dropMimeData()` to handle sample drops
- Uses sample IDs (not indices) to avoid stale reference issues
- Emits signals: `sampleDropped(sampleId, targetSubset)`, `sampleDroppedToRoot(sampleId)`

**Key Features:**
- Samples can be dragged between subsets
- Samples can be dragged from subsets back to root
- Samples can be dragged from root into subsets
- Visual drop indicators show valid drop zones
- Subsets cannot be dragged (only accept drops)

**Modified Files:**
- `src/gui/DatasetView.{h,cpp}` - Integrated DatasetTreeModel
- `src/gui/DatasetView.cpp:setupUI()` - Enabled drag-drop mode
- `src/gui/DatasetView.cpp:populateTree()` - Store sample IDs in UserRole+2

### 2. Batch Operations

**Multi-Selection Support:**
- Changed tree view selection mode from `SingleSelection` to `ExtendedSelection`
- Users can Ctrl+Click or Shift+Click to select multiple samples
- Context menu adapts based on selection count

**Batch Move to Subset:**
- New signal: `DatasetView::batchMoveToSubsetRequested(QList<int>)`
- New slot: `MainWindow::onBatchMoveToSubsetRequested()`
- Context menu shows "Move Selected to Subset..." when multiple samples selected
- Moves samples in reverse order to preserve indices during batch operation

**Implementation:**
```cpp
// DatasetView.cpp - Context menu logic
if (multipleSelected) {
    QAction* batchMoveAction = contextMenu.addAction("Move Selected to Subset...");
    // ...
}
```

**Modified Files:**
- `src/gui/DatasetView.h` - Added `getSelectedSampleIndices()`, `batchMoveToSubsetRequested` signal
- `src/gui/DatasetView.cpp` - Updated context menu to handle multiple selection
- `src/gui/MainWindow.h` - Added `onBatchMoveToSubsetRequested()` slot
- `src/gui/MainWindow.cpp` - Implemented batch move logic

### 3. Move Sample from Subset to Root

**Drag-Drop Implementation:**
- Samples can be dragged from subset back to invisible root
- Signal: `DatasetTreeModel::sampleDroppedToRoot(QString sampleId)`
- Handler: `MainWindow::onSampleDraggedToRoot(QString sampleId)`

**Logic Flow:**
1. User drags sample from subset to empty area or root level
2. Model emits `sampleDroppedToRoot` with sample ID
3. MainWindow finds which subset contains the sample
4. Calls `Dataset::moveSampleFromSubset(subsetName, index)`
5. View refreshes to show updated hierarchy

**Modified Files:**
- `src/gui/MainWindow.cpp` - Implemented `onSampleDraggedToRoot()` with subset search logic

### 4. Keyboard Shortcuts

**Implemented Shortcuts:**
- **Ctrl+I**: Import Files dialog
- **Ctrl+E**: Export Dataset dialog

**Implementation:**
```cpp
QAction* importAction = fileMenu->addAction(tr("&Import Files..."), 
    this, &MainWindow::onImportFiles);
importAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
```

**Modified Files:**
- `src/gui/MainWindow.cpp:createMenuBar()` - Added shortcut assignments

### 5. Sample ID-Based Operations

**Why Sample IDs Instead of Indices:**
- Indices become stale during drag-drop operations
- Samples can exist in different subsets
- Need stable identifiers across tree structure

**Implementation:**
- Tree items store sample ID in `Qt::UserRole + 2`
- Drag-drop MIME data includes sample IDs
- MainWindow handlers search for samples by ID

**Data Storage:**
```cpp
nameItem->setData(i, Qt::UserRole);                      // Index
nameItem->setData(sample.metadata().id, Qt::UserRole + 2); // ID
nameItem->setData(false, Qt::UserRole + 1);              // isSubset flag
```

## Technical Changes

### File Structure
```
src/gui/
├── DatasetTreeModel.h        [NEW] - Custom model with drag-drop
├── DatasetTreeModel.cpp      [NEW] - 100 lines
├── DatasetView.h             [MODIFIED] - Added batch operation methods
├── DatasetView.cpp           [MODIFIED] - Integrated DatasetTreeModel
├── MainWindow.h              [MODIFIED] - Added drag-drop handler slots
└── MainWindow.cpp            [MODIFIED] - Implemented all handlers
```

### Build System
- `CMakeLists.txt` - Added `DatasetTreeModel.cpp` to `GUI_SOURCES`

### Statistics
- **New Files**: 2 (DatasetTreeModel.h/cpp)
- **Modified Files**: 5 (DatasetView, MainWindow, CMakeLists.txt)
- **Lines Added**: ~400
- **New Dependencies**: QIODevice (for MIME data decoding)

## Testing

### Manual Testing Checklist

✅ **Drag-Drop Tests:**
- [x] Drag sample from root to subset
- [x] Drag sample between subsets
- [x] Drag sample from subset to root
- [x] Drop indicator shows correctly
- [x] Invalid drop targets are rejected

✅ **Batch Operation Tests:**
- [x] Select multiple samples with Ctrl+Click
- [x] Select range with Shift+Click
- [x] Context menu shows batch options
- [x] Batch move to subset works
- [x] Indices remain valid during batch operations

✅ **Keyboard Shortcut Tests:**
- [x] Ctrl+I opens import dialog
- [x] Ctrl+E opens export dialog
- [x] Shortcuts shown in menu

### Integration Test

```bash
# Test drag-drop and batch operations
./build/DatasetCreator
# 1. Import test_data files
# 2. Create "training" subset via context menu
# 3. Drag 3 samples to training subset
# 4. Create "validation" subset
# 5. Select 2 samples with Ctrl+Click
# 6. Right-click → "Move Selected to Subset..." → validation
# 7. Drag sample from validation back to root
# 8. Verify tree structure updates correctly
```

## User Experience Improvements

### Before Phase 4:
- ❌ Only context menu for moving samples
- ❌ Single sample operations only
- ❌ No visual feedback during moves
- ❌ No keyboard shortcuts
- ❌ Can't move samples back to root easily

### After Phase 4:
- ✅ Drag-and-drop for intuitive organization
- ✅ Multi-select for batch operations
- ✅ Visual drop indicators
- ✅ Ctrl+I / Ctrl+E shortcuts
- ✅ Flexible sample movement (to/from subsets)

## Performance Considerations

**Sample ID Lookups:**
- O(n) search through samples to find by ID
- Acceptable for datasets < 10K samples
- For larger datasets, consider:
  - Hash map cache: `QHash<QString, int> sampleIdToIndex_`
  - Update cache on `refresh()`

**Drag-Drop MIME Data:**
- Minimal overhead (IDs are strings, ~10-20 bytes each)
- No sample data copied during drag

**Batch Operations:**
- Reverse-order processing avoids index invalidation
- O(n log n) sort + O(n) moves = efficient

## Known Limitations

1. **No Undo/Redo**: Drag-drop and batch moves are immediate and irreversible
2. **No Visual Feedback**: No progress bar for batch moves (instant for small datasets)
3. **Subset Creation**: Still requires context menu (no drag-to-new-subset)
4. **No Drag Multiple**: Drag-drop works on single sample only (use batch move for multiple)

## Future Enhancements (Phase 5+)

**Deferred Features:**
- Subset distribution pie chart widget
- Automatic 70/20/10 split button
- Stratified sampling (balanced splits)
- Drag-and-drop file import
- Undo/Redo system

**Performance Optimizations:**
- Sample ID → index cache
- Incremental view updates (no full refresh)
- Virtual tree for large datasets

## API Changes

### New Public Methods

**DatasetView:**
```cpp
QString getSelectedSubsetName() const;
QList<int> getSelectedSampleIndices() const;
```

**DatasetView Signals:**
```cpp
void batchMoveToSubsetRequested(const QList<int>& sampleIndices);
void sampleDraggedToSubset(const QString& sampleId, const QString& subsetName);
void sampleDraggedToRoot(const QString& sampleId);
```

**MainWindow Slots:**
```cpp
void onBatchMoveToSubsetRequested(const QList<int>& sampleIndices);
void onSampleDraggedToSubset(const QString& sampleId, const QString& subsetName);
void onSampleDraggedToRoot(const QString& sampleId);
```

**DatasetTreeModel:**
```cpp
Qt::DropActions supportedDropActions() const override;
bool dropMimeData(...) override;
Qt::ItemFlags flags(const QModelIndex& index) const override;
```

## Backward Compatibility

✅ **Fully Compatible:**
- All Phase 3 features work unchanged
- Existing datasets load/save correctly
- Context menu operations still available
- Export formats unchanged

## Conclusion

Phase 4 successfully implements advanced workflow features that significantly improve the user experience for dataset organization. The drag-and-drop system is intuitive and responsive, batch operations enable efficient management of large datasets, and keyboard shortcuts speed up common tasks.

**Key Achievements:**
- ✅ Drag-and-drop sample organization
- ✅ Multi-select batch operations
- ✅ Bidirectional sample movement (root ↔ subsets)
- ✅ Keyboard shortcuts for common actions
- ✅ Robust ID-based sample tracking

**Next Steps:**
- Phase 5: Visual analytics and automation features
- Consider user feedback for UI refinements
- Performance testing with 10K+ sample datasets

---

**Commit Message:**
```
feat: add advanced workflow features with drag-and-drop

- Implement DatasetTreeModel with drag-drop support
- Add batch selection and batch move to subset
- Enable moving samples from subsets back to root
- Add keyboard shortcuts (Ctrl+I for import, Ctrl+E for export)
- Use sample IDs instead of indices for stable drag-drop operations
- Support multi-select with Ctrl+Click and Shift+Click

Phase 4 implementation complete: users can now efficiently organize
datasets using intuitive drag-and-drop and batch operations.
```
