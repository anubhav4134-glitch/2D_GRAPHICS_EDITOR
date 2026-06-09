#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
 
/* ─── Canvas ─────────────────────────────────────────── */
 
#define ROWS 30
#define COLS 60
#define MAX_OBJ 50
 
char canvas[ROWS][COLS];
 
void clear_canvas(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}
 
/* Safe single-cell write */
void plot(int x, int y, char ch) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS)
        canvas[y][x] = ch;
}
 
void display_canvas(void) {
    char border[COLS + 3];
    border[0] = '+';
    for (int i = 1; i <= COLS; i++) border[i] = '-';
    border[COLS + 1] = '+';
    border[COLS + 2] = '\0';
 
    printf("%s\n", border);
    for (int r = 0; r < ROWS; r++) {
        printf("|");
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }
    printf("%s\n", border);
}
 
/* ─── Drawing primitives ─────────────────────────────── */
 
void draw_rectangle(int x, int y, int w, int h) {
    for (int i = x; i < x + w; i++) {
        plot(i, y,         '_');
        plot(i, y + h - 1, '_');
    }
    for (int j = y; j < y + h; j++) {
        plot(x,         j, '*');
        plot(x + w - 1, j, '*');
    }
    plot(x,         y,         '*');
    plot(x + w - 1, y,         '*');
    plot(x,         y + h - 1, '*');
    plot(x + w - 1, y + h - 1, '*');
}
 
void draw_circle(int cx, int cy, int r) {
    for (int deg = 0; deg < 360; deg++) {
        double rad = deg * 3.14159265 / 180.0;
        int px = (int)round(cx + r * cos(rad) * 2.0);
        int py = (int)round(cy + r * sin(rad) * 0.5);
        plot(px, py, '*');
    }
}
 
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int x = x1, y = y1;
    int horiz = (dy == 0);
 
    while (1) {
        plot(x, y, horiz ? '_' : '*');
        if (x == x2 && y == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 <  dx) { err += dx; y += sy; }
    }
}
 
void draw_triangle(int x, int y, int base, int height) {
    int apex_x = x + base / 2;
    for (int row = 0; row < height; row++) {
        int spread = (height > 1)
                     ? (int)round((double)row * (base / 2.0) / (height - 1))
                     : 0;
        int lx = apex_x - spread;
        int rx = apex_x + spread;
 
        if (row == height - 1) {
            for (int i = lx; i <= rx; i++)
                plot(i, y + row, '_');
        } else {
            plot(lx, y + row, '*');
            plot(rx, y + row, '*');
        }
    }
    plot(apex_x, y, '*');
}
 
/* ─── Object store ───────────────────────────────────── */
 
typedef enum { RECTANGLE, CIRCLE, LINE, TRIANGLE } ShapeType;
 
typedef struct { int x, y, w, h;            } RectParams;
typedef struct { int cx, cy, r;              } CircleParams;
typedef struct { int x1, y1, x2, y2;        } LineParams;
typedef struct { int x, y, base, height;    } TriParams;
 
typedef union {
    RectParams   rect;
    CircleParams circle;
    LineParams   line;
    TriParams    tri;
} Params;
 
typedef struct {
    int       id;
    ShapeType type;
    Params    p;
    int       active;
} Object;
 
Object objects[MAX_OBJ];
int obj_count = 0;
int next_id   = 1;
 
int find_obj(int id) {
    for (int i = 0; i < obj_count; i++)
        if (objects[i].active && objects[i].id == id)
            return i;
    return -1;
}
 
/* ─── Add ────────────────────────────────────────────── */
 
int add_rectangle(int x, int y, int w, int h) {
    if (obj_count >= MAX_OBJ) { printf("Object limit reached.\n"); return -1; }
    Object *o = &objects[obj_count++];
    o->id = next_id++; o->type = RECTANGLE; o->active = 1;
    o->p.rect = (RectParams){x, y, w, h};
    printf("[add] Rectangle id=%d  x=%d y=%d w=%d h=%d\n", o->id, x, y, w, h);
    return o->id;
}
 
int add_circle(int cx, int cy, int r) {
    if (obj_count >= MAX_OBJ) { printf("Object limit reached.\n"); return -1; }
    Object *o = &objects[obj_count++];
    o->id = next_id++; o->type = CIRCLE; o->active = 1;
    o->p.circle = (CircleParams){cx, cy, r};
    printf("[add] Circle    id=%d  cx=%d cy=%d r=%d\n", o->id, cx, cy, r);
    return o->id;
}
 
int add_line(int x1, int y1, int x2, int y2) {
    if (obj_count >= MAX_OBJ) { printf("Object limit reached.\n"); return -1; }
    Object *o = &objects[obj_count++];
    o->id = next_id++; o->type = LINE; o->active = 1;
    o->p.line = (LineParams){x1, y1, x2, y2};
    printf("[add] Line      id=%d  (%d,%d)->(%d,%d)\n", o->id, x1, y1, x2, y2);
    return o->id;
}
 
int add_triangle(int x, int y, int base, int height) {
    if (obj_count >= MAX_OBJ) { printf("Object limit reached.\n"); return -1; }
    Object *o = &objects[obj_count++];
    o->id = next_id++; o->type = TRIANGLE; o->active = 1;
    o->p.tri = (TriParams){x, y, base, height};
    printf("[add] Triangle  id=%d  x=%d y=%d base=%d h=%d\n", o->id, x, y, base, height);
    return o->id;
}
 
/* ─── Delete ─────────────────────────────────────────── */
 
void delete_object(int id) {
    int i = find_obj(id);
    if (i < 0) { printf("[delete] No object with id=%d.\n", id); return; }
    objects[i].active = 0;
    printf("[delete] Object id=%d removed.\n", id);
}
 
/* ─── Modify ─────────────────────────────────────────── */
 
void modify_rectangle(int id, int x, int y, int w, int h) {
    int i = find_obj(id);
    if (i < 0 || objects[i].type != RECTANGLE) {
        printf("[modify] Rectangle id=%d not found.\n", id); return;
    }
    objects[i].p.rect = (RectParams){x, y, w, h};
    printf("[modify] Rectangle id=%d updated.\n", id);
}
 
void modify_circle(int id, int cx, int cy, int r) {
    int i = find_obj(id);
    if (i < 0 || objects[i].type != CIRCLE) {
        printf("[modify] Circle id=%d not found.\n", id); return;
    }
    objects[i].p.circle = (CircleParams){cx, cy, r};
    printf("[modify] Circle id=%d updated.\n", id);
}
 
void modify_line(int id, int x1, int y1, int x2, int y2) {
    int i = find_obj(id);
    if (i < 0 || objects[i].type != LINE) {
        printf("[modify] Line id=%d not found.\n", id); return;
    }
    objects[i].p.line = (LineParams){x1, y1, x2, y2};
    printf("[modify] Line id=%d updated.\n", id);
}
 
void modify_triangle(int id, int x, int y, int base, int height) {
    int i = find_obj(id);
    if (i < 0 || objects[i].type != TRIANGLE) {
        printf("[modify] Triangle id=%d not found.\n", id); return;
    }
    objects[i].p.tri = (TriParams){x, y, base, height};
    printf("[modify] Triangle id=%d updated.\n", id);
}
 
/* ─── List ───────────────────────────────────────────── */
 
void list_objects(void) {
    printf("\n%-4s  %-10s  Details\n", "ID", "Type");
    printf("--------------------------------------------------\n");
    int found = 0;
    for (int i = 0; i < obj_count; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Object *o = &objects[i];
        printf("%-4d  ", o->id);
        switch (o->type) {
            case RECTANGLE:
                printf("%-10s  x=%d y=%d w=%d h=%d\n", "Rectangle",
                    o->p.rect.x, o->p.rect.y, o->p.rect.w, o->p.rect.h); break;
            case CIRCLE:
                printf("%-10s  cx=%d cy=%d r=%d\n", "Circle",
                    o->p.circle.cx, o->p.circle.cy, o->p.circle.r); break;
            case LINE:
                printf("%-10s  (%d,%d)->(%d,%d)\n", "Line",
                    o->p.line.x1, o->p.line.y1, o->p.line.x2, o->p.line.y2); break;
            case TRIANGLE:
                printf("%-10s  x=%d y=%d base=%d h=%d\n", "Triangle",
                    o->p.tri.x, o->p.tri.y, o->p.tri.base, o->p.tri.height); break;
        }
    }
    if (!found) printf("  (no objects)\n");
    printf("\n");
}
 
/* ─── Render ─────────────────────────────────────────── */
 
void render_and_display(void) {
    clear_canvas();
    for (int i = 0; i < obj_count; i++) {
        if (!objects[i].active) continue;
        Object *o = &objects[i];
        switch (o->type) {
            case RECTANGLE:
                draw_rectangle(o->p.rect.x, o->p.rect.y,
                               o->p.rect.w, o->p.rect.h);   break;
            case CIRCLE:
                draw_circle(o->p.circle.cx, o->p.circle.cy,
                            o->p.circle.r);                  break;
            case LINE:
                draw_line(o->p.line.x1, o->p.line.y1,
                          o->p.line.x2, o->p.line.y2);      break;
            case TRIANGLE:
                draw_triangle(o->p.tri.x,    o->p.tri.y,
                              o->p.tri.base, o->p.tri.height); break;
        }
    }
    display_canvas();
}
 
/* ─── Interactive menu ───────────────────────────────── */
 
void flush_stdin(void) { int c; while ((c = getchar()) != '\n' && c != EOF); }
 
void menu(void) {
    int choice;
    while (1) {
        printf("\n===== 2D Graphics Editor =====\n");
        printf(" 1. Add Rectangle\n");
        printf(" 2. Add Circle\n");
        printf(" 3. Add Line\n");
        printf(" 4. Add Triangle\n");
        printf(" 5. Delete Object\n");
        printf(" 6. Modify Rectangle\n");
        printf(" 7. Modify Circle\n");
        printf(" 8. Modify Line\n");
        printf(" 9. Modify Triangle\n");
        printf("10. Display Picture\n");
        printf("11. List Objects\n");
        printf(" 0. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        flush_stdin();
 
        int id, x, y, w, h, r, cx, cy, x1, y1, x2, y2, base, height;
 
        switch (choice) {
            case 1:
                printf("Enter x y width height: ");
                scanf("%d %d %d %d", &x, &y, &w, &h);
                add_rectangle(x, y, w, h); break;
            case 2:
                printf("Enter cx cy radius: ");
                scanf("%d %d %d", &cx, &cy, &r);
                add_circle(cx, cy, r); break;
            case 3:
                printf("Enter x1 y1 x2 y2: ");
                scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
                add_line(x1, y1, x2, y2); break;
            case 4:
                printf("Enter x y base height: ");
                scanf("%d %d %d %d", &x, &y, &base, &height);
                add_triangle(x, y, base, height); break;
            case 5:
                printf("Object id to delete: ");
                scanf("%d", &id);
                delete_object(id); break;
            case 6:
                printf("Enter id x y width height: ");
                scanf("%d %d %d %d %d", &id, &x, &y, &w, &h);
                modify_rectangle(id, x, y, w, h); break;
            case 7:
                printf("Enter id cx cy radius: ");
                scanf("%d %d %d %d", &id, &cx, &cy, &r);
                modify_circle(id, cx, cy, r); break;
            case 8:
                printf("Enter id x1 y1 x2 y2: ");
                scanf("%d %d %d %d %d", &id, &x1, &y1, &x2, &y2);
                modify_line(id, x1, y1, x2, y2); break;
            case 9:
                printf("Enter id x y base height: ");
                scanf("%d %d %d %d %d", &id, &x, &y, &base, &height);
                modify_triangle(id, x, y, base, height); break;
            case 10:
                render_and_display(); break;
            case 11:
                list_objects(); break;
            case 0:
                printf("Goodbye!\n"); return;
            default:
                printf("Invalid choice.\n");
        }
    }
}
 
/* ─── Main ───────────────────────────────────────────── */
 
int main(void) {
    add_rectangle(2,  1,  20, 8);
    add_circle(38, 10, 7);
    add_triangle(4, 14, 20, 9);
    add_line(26, 24, 55, 24);
 
    printf("\n--- Initial picture ---\n");
    render_and_display();
 
    menu();
    return 0;
}