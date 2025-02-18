/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Forward.h>
#include <LibWeb/Layout/FormattingContext.h>
#include <LibWeb/Layout/TableWrapper.h>

namespace Web::Layout {

enum class TableDimension {
    Row,
    Column
};

class TableFormattingContext final : public FormattingContext {
public:
    explicit TableFormattingContext(LayoutState&, Box const&, FormattingContext* parent);
    ~TableFormattingContext();

    virtual void run(Box const&, LayoutMode, AvailableSpace const&) override;
    virtual CSSPixels automatic_content_width() const override;
    virtual CSSPixels automatic_content_height() const override;

    Box const& table_box() const { return context_box(); }
    TableWrapper const& table_wrapper() const
    {
        return verify_cast<TableWrapper>(*table_box().containing_block());
    }

    static bool border_is_less_specific(const CSS::BorderData& a, const CSS::BorderData& b);

private:
    CSSPixels run_caption_layout(LayoutMode, CSS::CaptionSide);
    CSSPixels compute_capmin();
    void calculate_row_column_grid(Box const&);
    void compute_constrainedness();
    void compute_cell_measures(AvailableSpace const& available_space);
    void compute_outer_content_sizes();
    template<class RowOrColumn>
    void initialize_table_measures();
    template<class RowOrColumn>
    void compute_table_measures();
    template<class RowOrColumn>
    void compute_intrinsic_percentage(size_t max_cell_span);
    void compute_table_width();
    void distribute_width_to_columns();
    void distribute_excess_width_to_columns(CSSPixels available_width);
    void compute_table_height(LayoutMode layout_mode);
    void distribute_height_to_rows();
    void position_row_boxes();
    void position_cell_boxes();
    void border_conflict_resolution();
    CSSPixels border_spacing_horizontal() const;
    CSSPixels border_spacing_vertical() const;

    CSSPixels compute_columns_total_used_width() const;
    void commit_candidate_column_widths(Vector<CSSPixels> const& candidate_widths);
    void assign_columns_width_linear_combination(Vector<CSSPixels> const& candidate_widths, CSSPixels available_width);

    template<class ColumnFilter>
    bool distribute_excess_width_proportionally_to_max_width(CSSPixels excess_width, ColumnFilter column_filter);
    template<class ColumnFilter>
    bool distribute_excess_width_equally(CSSPixels excess_width, ColumnFilter column_filter);
    template<class ColumnFilter>
    bool distribute_excess_width_by_intrinsic_percentage(CSSPixels excess_width, ColumnFilter column_filter);

    CSSPixels m_table_height { 0 };
    CSSPixels m_automatic_content_height { 0 };

    Optional<AvailableSpace> m_available_space;

    struct Column {
        CSSPixels left_offset { 0 };
        CSSPixels min_size { 0 };
        CSSPixels max_size { 0 };
        CSSPixels used_width { 0 };
        bool has_intrinsic_percentage { false };
        double intrinsic_percentage { 0 };
        // Store whether the column is constrained: https://www.w3.org/TR/css-tables-3/#constrainedness
        bool is_constrained { false };
        // Store whether the column has originating cells, defined in https://www.w3.org/TR/css-tables-3/#terminology.
        bool has_originating_cells { false };
    };

    struct Row {
        JS::NonnullGCPtr<Box const> box;
        CSSPixels base_height { 0 };
        CSSPixels reference_height { 0 };
        CSSPixels final_height { 0 };
        CSSPixels baseline { 0 };
        CSSPixels min_size { 0 };
        CSSPixels max_size { 0 };
        bool has_intrinsic_percentage { false };
        double intrinsic_percentage { 0 };
        // Store whether the row is constrained: https://www.w3.org/TR/css-tables-3/#constrainedness
        bool is_constrained { false };
    };

    struct Cell {
        JS::NonnullGCPtr<Box const> box;
        size_t column_index;
        size_t row_index;
        size_t column_span;
        size_t row_span;
        CSSPixels baseline { 0 };
        CSSPixels outer_min_width { 0 };
        CSSPixels outer_max_width { 0 };
        CSSPixels outer_min_height { 0 };
        CSSPixels outer_max_height { 0 };
    };

    // Accessors to enable direction-agnostic table measurement.

    template<class RowOrColumn>
    static size_t cell_span(Cell const& cell);

    template<class RowOrColumn>
    static size_t cell_index(Cell const& cell);

    template<class RowOrColumn>
    static CSSPixels cell_min_size(Cell const& cell);

    template<class RowOrColumn>
    static CSSPixels cell_max_size(Cell const& cell);

    template<class RowOrColumn>
    static double cell_percentage_contribution(Cell const& cell);

    template<class RowOrColumn>
    static bool cell_has_intrinsic_percentage(Cell const& cell);

    template<class RowOrColumn>
    void initialize_intrinsic_percentages_from_rows_or_columns();

    template<class RowOrColumn>
    void initialize_intrinsic_percentages_from_cells();

    template<class RowOrColumn>
    CSSPixels border_spacing();

    template<class RowOrColumn>
    Vector<RowOrColumn>& table_rows_or_columns();

    CSSPixels compute_row_content_height(Cell const& cell) const;

    enum class ConflictingSide {
        Top,
        Bottom,
        Left,
        Right,
    };

    struct ConflictingEdge {
        Node const* element;
        ConflictingSide side;
    };

    static const CSS::BorderData& border_data_conflicting_edge(ConflictingEdge const& conflicting_edge);

    class BorderConflictFinder {
    public:
        BorderConflictFinder(TableFormattingContext const* context);
        Vector<ConflictingEdge> conflicting_edges(Cell const&, ConflictingSide) const;

    private:
        void collect_conflicting_col_elements();
        void collect_conflicting_row_group_elements();

        struct RowGroupInfo {
            Node const* row_group;
            size_t start_index;
            size_t row_count;
        };

        Vector<Node const*> m_col_elements_by_index;
        Vector<Optional<RowGroupInfo>> m_row_group_elements_by_index;
        TableFormattingContext const* m_context;
    };

    Vector<Cell> m_cells;
    Vector<Vector<Optional<Cell const&>>> m_cells_by_coordinate;
    Vector<Column> m_columns;
    Vector<Row> m_rows;
};

}
