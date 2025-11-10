#include "unity.h"
#include "ast.h"
#include <string.h>
#include <stdbool.h>

/* Helpers */
static inline SrcSpan SPAN(int ls, int cs, int le, int ce) {
    SrcSpan s = { ls, cs, le, ce };
    return s;
}

#define START_ARENA() AstArena *A = ast_arena_new(); TEST_ASSERT_NOT_NULL(A)
#define END_ARENA()   ast_arena_free(A)


/* ========== TESTS ========== */

void test_arena_alloc_and_ast_new(void) {
    START_ARENA();
    SrcSpan s = SPAN(1,1,1,5);
    AstNode *n = ast_new(A, AST_IDENT, s);
    TEST_ASSERT_NOT_NULL(n);
    TEST_ASSERT_EQUAL_INT(AST_IDENT, n->kind);
    TEST_ASSERT_EQUAL_INT(1, n->span.line_start);
    TEST_ASSERT_EQUAL_INT(5, n->span.col_end);
    END_ARENA();
}

void test_ast_list_push_growth_preserves_order(void) {
    START_ARENA();
    AstList list = {0};
    /* Insertar 50 nodos para forzar varios crecimientos */
    const int N = 50;
    for (int i = 0; i < N; i++) {
        AstNode *lit = ast_new(A, AST_LITERAL, SPAN(1,i+1,1,i+1));
        lit->as.literal.tag = LIT_INT;
        lit->as.literal.val.i = i;
        ast_list_push(A, &list, lit);
    }
    TEST_ASSERT_EQUAL_INT(N, (int)list.count);
    for (int i = 0; i < N; i++) {
        AstNode *lit = list.data[i];
        TEST_ASSERT_EQUAL_INT(AST_LITERAL, lit->kind);
        TEST_ASSERT_EQUAL_INT(LIT_INT, lit->as.literal.tag);
        TEST_ASSERT_EQUAL_INT(i, (int)lit->as.literal.val.i);
    }
    END_ARENA();
}

void test_build_binop_tree_precedence_add_mul(void) {
    START_ARENA();
    /* a + b * c  =>  add(a, mul(b, c)) */
    AstNode *id_a = ast_new(A, AST_IDENT, SPAN(1,1,1,1)); id_a->as.ident.lexeme = "a";
    AstNode *id_b = ast_new(A, AST_IDENT, SPAN(1,5,1,5)); id_b->as.ident.lexeme = "b";
    AstNode *id_c = ast_new(A, AST_IDENT, SPAN(1,9,1,9)); id_c->as.ident.lexeme = "c";

    AstNode *mul = ast_new(A, AST_BINOP, SPAN(1,7,1,9));
    mul->as.binop.op = OP_MUL;
    mul->as.binop.left = id_b;
    mul->as.binop.right = id_c;

    AstNode *add = ast_new(A, AST_BINOP, SPAN(1,1,1,9));
    add->as.binop.op = OP_ADD;
    add->as.binop.left = id_a;
    add->as.binop.right = mul;

    TEST_ASSERT_EQUAL_INT(AST_BINOP, add->kind);
    TEST_ASSERT_EQUAL_INT(OP_ADD, add->as.binop.op);
    TEST_ASSERT_EQUAL_PTR(id_a, add->as.binop.left);
    TEST_ASSERT_EQUAL_INT(AST_BINOP, add->as.binop.right->kind);
    TEST_ASSERT_EQUAL_INT(OP_MUL, add->as.binop.right->as.binop.op);
    TEST_ASSERT_EQUAL_PTR(id_b, add->as.binop.right->as.binop.left);
    TEST_ASSERT_EQUAL_PTR(id_c, add->as.binop.right->as.binop.right);
    END_ARENA();
}

void test_if_with_elseifs_and_else_structure(void) {
    START_ARENA();
    /* If c1 Then B1 ElseIf c2 Then B2 Else B3 End */

    AstNode *c1 = ast_new(A, AST_IDENT, SPAN(1,4,1,5)); c1->as.ident.lexeme = "c1";
    AstNode *c2 = ast_new(A, AST_IDENT, SPAN(2,10,2,11)); c2->as.ident.lexeme = "c2";

    AstNode *B1 = ast_new(A, AST_BLOCK, SPAN(1,7,1,15));
    AstNode *B2 = ast_new(A, AST_BLOCK, SPAN(2,14,2,20));
    AstNode *B3 = ast_new(A, AST_BLOCK, SPAN(3,7,3,10));

    AstNode *ifn = ast_new(A, AST_IF, SPAN(1,1,4,3));
    ifn->as.if_stmt.cond = c1;
    ifn->as.if_stmt.then_blk = B1;

    /* modelamos un elseif como un par cond+blk dentro de un mini-IF */
    AstNode *elseif1 = ast_new(A, AST_IF, SPAN(2,1,2,20));
    elseif1->as.if_stmt.cond = c2;
    elseif1->as.if_stmt.then_blk = B2;

    ast_list_push(A, &ifn->as.if_stmt.elseifs, elseif1);
    ifn->as.if_stmt.else_blk = B3;

    TEST_ASSERT_EQUAL_INT(AST_IF, ifn->kind);
    TEST_ASSERT_EQUAL_PTR(c1, ifn->as.if_stmt.cond);
    TEST_ASSERT_EQUAL_PTR(B1, ifn->as.if_stmt.then_blk);
    TEST_ASSERT_EQUAL_INT(1, (int)ifn->as.if_stmt.elseifs.count);
    TEST_ASSERT_EQUAL_INT(AST_IF, ifn->as.if_stmt.elseifs.data[0]->kind);
    TEST_ASSERT_EQUAL_PTR(B3, ifn->as.if_stmt.else_blk);
    END_ARENA();
}

void test_assign_node_right_associative_shape(void) {
    START_ARENA();
    /* a = b = c  →  Assign(a, Assign(b, c)) (estructura del árbol, semántica aparte) */
    AstNode *a = ast_new(A, AST_IDENT, SPAN(1,1,1,1)); a->as.ident.lexeme = "a";
    AstNode *b = ast_new(A, AST_IDENT, SPAN(1,5,1,5)); b->as.ident.lexeme = "b";
    AstNode *c = ast_new(A, AST_IDENT, SPAN(1,9,1,9)); c->as.ident.lexeme = "c";

    AstNode *assign_bc = ast_new(A, AST_ASSIGN, SPAN(1,5,1,9));
    assign_bc->as.assign.op = OP_ASSIGN;
    assign_bc->as.assign.lhs = b;
    assign_bc->as.assign.rhs = c;

    AstNode *assign_a_bc = ast_new(A, AST_ASSIGN, SPAN(1,1,1,9));
    assign_a_bc->as.assign.op = OP_ASSIGN;
    assign_a_bc->as.assign.lhs = a;
    assign_a_bc->as.assign.rhs = assign_bc;

    TEST_ASSERT_EQUAL_INT(AST_ASSIGN, assign_a_bc->kind);
    TEST_ASSERT_EQUAL_PTR(a, assign_a_bc->as.assign.lhs);
    TEST_ASSERT_EQUAL_INT(AST_ASSIGN, assign_a_bc->as.assign.rhs->kind);
    TEST_ASSERT_EQUAL_PTR(b, assign_a_bc->as.assign.rhs->as.assign.lhs);
    TEST_ASSERT_EQUAL_PTR(c, assign_a_bc->as.assign.rhs->as.assign.rhs);
    END_ARENA();
}

void test_call_and_index_nodes(void) {
    START_ARENA();
    /* f(x, y[0]) */
    AstNode *f = ast_new(A, AST_IDENT, SPAN(1,1,1,1)); f->as.ident.lexeme = "f";
    AstNode *x = ast_new(A, AST_IDENT, SPAN(1,3,1,3)); x->as.ident.lexeme = "x";
    AstNode *y = ast_new(A, AST_IDENT, SPAN(1,6,1,6)); y->as.ident.lexeme = "y";
    AstNode *zero = ast_new(A, AST_LITERAL, SPAN(1,8,1,8));
    zero->as.literal.tag = LIT_INT; zero->as.literal.val.i = 0;

    AstNode *y0 = ast_new(A, AST_INDEX, SPAN(1,6,1,8));
    y0->as.index.base = y;
    y0->as.index.index = zero;

    AstNode *call = ast_new(A, AST_CALL, SPAN(1,1,1,9));
    call->as.call.callee = f;
    ast_list_push(A, &call->as.call.args, x);
    ast_list_push(A, &call->as.call.args, y0);

    TEST_ASSERT_EQUAL_INT(AST_CALL, call->kind);
    TEST_ASSERT_EQUAL_PTR(f, call->as.call.callee);
    TEST_ASSERT_EQUAL_INT(2, (int)call->as.call.args.count);
    TEST_ASSERT_EQUAL_INT(AST_IDENT, call->as.call.args.data[0]->kind);
    TEST_ASSERT_EQUAL_INT(AST_INDEX, call->as.call.args.data[1]->kind);
    END_ARENA();
}

void test_literal_variants_int_float_string_bool_null(void) {
    START_ARENA();
    AstNode *li = ast_new(A, AST_LITERAL, SPAN(1,1,1,1));
    li->as.literal.tag = LIT_INT; li->as.literal.val.i = 42;
    TEST_ASSERT_EQUAL_INT(LIT_INT, li->as.literal.tag);
    TEST_ASSERT_EQUAL_INT(42, (int)li->as.literal.val.i);

    AstNode *lf = ast_new(A, AST_LITERAL, SPAN(1,1,1,1));
    lf->as.literal.tag = LIT_FLOAT; lf->as.literal.val.f = 3.5;
    TEST_ASSERT_EQUAL_INT(LIT_FLOAT, lf->as.literal.tag);
    TEST_ASSERT_FLOAT_WITHIN(1e-9, 3.5, lf->as.literal.val.f);

    AstNode *ls = ast_new(A, AST_LITERAL, SPAN(1,1,1,3));
    ls->as.literal.tag = LIT_STRING; ls->as.literal.val.s = "hi";
    TEST_ASSERT_EQUAL_INT(LIT_STRING, ls->as.literal.tag);
    TEST_ASSERT_EQUAL_STRING("hi", ls->as.literal.val.s);

    AstNode *lb = ast_new(A, AST_LITERAL, SPAN(1,1,1,1));
    lb->as.literal.tag = LIT_BOOL; lb->as.literal.val.b = true;
    TEST_ASSERT_EQUAL_INT(LIT_BOOL, lb->as.literal.tag);
    TEST_ASSERT_TRUE(lb->as.literal.val.b);

    AstNode *ln = ast_new(A, AST_LITERAL, SPAN(1,1,1,1));
    ln->as.literal.tag = LIT_NULL;
    TEST_ASSERT_EQUAL_INT(LIT_NULL, ln->as.literal.tag);
    END_ARENA();
}
