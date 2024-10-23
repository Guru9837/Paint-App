#include <wx/wx.h>
#include <vector>
#include <cstdlib> // For random color

// Base class for shapes
class Shape {
public:
    virtual void Draw(wxDC& dc) = 0; // Pure virtual function for drawing
    virtual ~Shape() {}
    virtual void SetColor(const wxColor& color) = 0; // Set color for the shape
};

// Circle class (static, no pulsing)
class Circle : public Shape {
private:
    wxPoint center;
    int radius;
    wxColor color;

public:
    Circle(const wxPoint& center, int radius, const wxColor& color)
        : center(center), radius(radius), color(color) {}

    void Draw(wxDC& dc) override {
        dc.SetBrush(wxBrush(color));
        dc.DrawCircle(center, radius);
    }

    void SetColor(const wxColor& color) override {
        this->color = color;
    }
};

// Square class
class Square : public Shape {
private:
    wxPoint topLeft;
    int sideLength;
    wxColor color;

public:
    Square(const wxPoint& topLeft, int sideLength, const wxColor& color)
        : topLeft(topLeft), sideLength(sideLength), color(color) {}

    void Draw(wxDC& dc) override {
        dc.SetBrush(wxBrush(color));
        dc.DrawRectangle(topLeft, wxSize(sideLength, sideLength));
    }

    void SetColor(const wxColor& color) override {
        this->color = color;
    }
};

// Freehand line class
class FreehandLine : public Shape {
private:
    std::vector<wxPoint> points;
    wxColor color;
    bool rainbowMode; // Enable rainbow mode for dynamic color changes

public:
    FreehandLine(const wxColor& color, bool rainbowMode = false) : color(color), rainbowMode(rainbowMode) {}

    void AddPoint(const wxPoint& point) {
        points.push_back(point);
    }

    void Draw(wxDC& dc) override {
        dc.SetPen(wxPen(color, 2)); // Set the pen color and width
        if (points.size() > 1) {
            dc.DrawLines(points.size(), points.data());
        }
    }

    void SetColor(const wxColor& color) override {
        this->color = color;
    }

    // Dynamically change color in rainbow mode
    void UpdateRainbowColor() {
        if (rainbowMode) {
            color = wxColor(rand() % 256, rand() % 256, rand() % 256); // Random RGB values
        }
    }
};

// Canvas class
class PaintCanvas : public wxPanel {
private:
    std::vector<Shape*> shapes;
    FreehandLine* currentLine = nullptr;
    Circle* currentCircle = nullptr;
    Square* currentSquare = nullptr;
    wxColor currentColor;
    bool rainbowMode = false;
    bool eraserMode = false;
    bool circleMode = false;  // Mode for drawing circles
    bool squareMode = false;  // Mode for drawing squares
    int shapeSize = 50;       // Default size for circles and squares

public:
    PaintCanvas(wxWindow* parent) : wxPanel(parent) {
        currentColor = *wxBLACK; // Default color

        Bind(wxEVT_PAINT, &PaintCanvas::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &PaintCanvas::OnLeftDown, this);
        Bind(wxEVT_LEFT_UP, &PaintCanvas::OnLeftUp, this);
        Bind(wxEVT_MOTION, &PaintCanvas::OnMouseMove, this);
    }

    ~PaintCanvas() {
        for (Shape* shape : shapes) {
            delete shape; // Clean up allocated memory
        }
        delete currentLine;
        delete currentCircle;
        delete currentSquare;
    }

    void OnPaint(wxPaintEvent& event) {
        wxPaintDC dc(this);
        for (Shape* shape : shapes) {
            shape->Draw(dc);
        }
        if (currentLine) {
            currentLine->Draw(dc); // Draw the current freehand line
        }
        if (currentCircle) {
            currentCircle->Draw(dc); // Draw the current circle
        }
        if (currentSquare) {
            currentSquare->Draw(dc); // Draw the current square
        }
    }

    void OnLeftDown(wxMouseEvent& event) {
        if (circleMode) {
            // Create a new circle at the clicked position with a fixed radius
            currentCircle = new Circle(event.GetPosition(), shapeSize, currentColor);
            shapes.push_back(currentCircle);
            currentCircle = nullptr; // Reset the current circle
            Refresh();
        }
        else if (squareMode) {
            // Create a new square at the clicked position with a fixed size
            currentSquare = new Square(event.GetPosition(), shapeSize, currentColor);
            shapes.push_back(currentSquare);
            currentSquare = nullptr; // Reset the current square
            Refresh();
        }
        else if (eraserMode) {
            currentLine = new FreehandLine(*wxWHITE); // Eraser draws with white color
        }
        else {
            currentLine = new FreehandLine(currentColor, rainbowMode);
        }
        if (currentLine) {
            currentLine->AddPoint(event.GetPosition());
        }
        Refresh();
    }

    void OnLeftUp(wxMouseEvent& event) {
        if (currentLine) {
            currentLine->AddPoint(event.GetPosition());
            shapes.push_back(currentLine); // Save the line to shapes
            currentLine = nullptr; // Reset current line
        }
        Refresh();
    }

    void OnMouseMove(wxMouseEvent& event) {
        if (currentLine) {
            if (rainbowMode) {
                currentLine->UpdateRainbowColor(); // Update rainbow color during drawing
            }
            currentLine->AddPoint(event.GetPosition());
            Refresh(); // Update drawing while dragging
        }
    }

    void SetColor(const wxColor& color) {
        currentColor = color;
        eraserMode = false; // Disable eraser mode when color is set
        rainbowMode = false; // Disable rainbow mode when a specific color is set
        circleMode = false;  // Disable circle mode when a specific color is set
        squareMode = false;  // Disable square mode when a specific color is set
    }

    void EnableRainbowMode() {
        rainbowMode = true;
        circleMode = false;  // Disable circle mode when rainbow is enabled
        squareMode = false;  // Disable square mode when rainbow is enabled
    }

    void EnableEraserMode() {
        eraserMode = true;
        circleMode = false;  // Disable circle mode when eraser is enabled
        squareMode = false;  // Disable square mode when eraser is enabled
    }

    void EnableCircleMode() {
        circleMode = true;   // Enable circle mode
        eraserMode = false;  // Disable other modes
        rainbowMode = false;
        squareMode = false;  // Disable square mode when circle is enabled
    }

    void EnableSquareMode() {
        squareMode = true;   // Enable square mode
        eraserMode = false;  // Disable other modes
        rainbowMode = false;
        circleMode = false;  // Disable circle mode when square is enabled
    }
};

// Application class
class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

// Custom IDs for color, shape, and modes
const int ID_COLOR_RED = wxID_HIGHEST + 1;
const int ID_COLOR_GREEN = wxID_HIGHEST + 2;
const int ID_COLOR_BLUE = wxID_HIGHEST + 3;
const int ID_MODE_RAINBOW = wxID_HIGHEST + 4;
const int ID_MODE_ERASER = wxID_HIGHEST + 5;
const int ID_MODE_CIRCLE = wxID_HIGHEST + 6;
const int ID_MODE_SQUARE = wxID_HIGHEST + 7; // New menu ID for square mode

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Interactive Paint App", wxDefaultPosition, wxSize(800, 600));
    PaintCanvas* canvas = new PaintCanvas(frame);

    wxMenuBar* menuBar = new wxMenuBar;

    // Color menu
    wxMenu* colorMenu = new wxMenu;
    colorMenu->Append(ID_COLOR_RED, "Red");
    colorMenu->Append(ID_COLOR_GREEN, "Green");
    colorMenu->Append(ID_COLOR_BLUE, "Blue");
    menuBar->Append(colorMenu, "Colors");

    // Fun modes menu
    wxMenu* modeMenu = new wxMenu;
    modeMenu->Append(ID_MODE_RAINBOW, "Rainbow Brush");
    modeMenu->Append(ID_MODE_ERASER, "Eraser");
    modeMenu->Append(ID_MODE_CIRCLE, "Draw Circle");
    modeMenu->Append(ID_MODE_SQUARE, "Draw Square");  // New menu option for square mode
    menuBar->Append(modeMenu, "Fun Modes");

    frame->SetMenuBar(menuBar);

    // Bind color selection events
    frame->Bind(wxEVT_MENU, [canvas](wxCommandEvent&) { canvas->SetColor(*wxRED); }, ID_COLOR_RED);
    frame->Bind(wxEVT_MENU, [canvas](wxCommandEvent&) { canvas->SetColor(*wxGREEN); }, ID_COLOR_GREEN);
    frame->Bind(wxEVT_MENU, [canvas](wxCommandEvent&) { canvas->SetColor(*wxBLUE); }, ID_COLOR_BLUE);

    // Bind mode selection events
    frame->Bind(wxEVT_MENU, [canvas](wxCommandEvent&) { canvas->EnableRainbowMode(); }, ID_MODE_RAINBOW);
    frame->Bind(wxEVT_MENU, [canvas](wxCommandEvent&) { canvas->EnableEraserMode(); }, ID_MODE_ERASER);
    frame->Bind(wxEVT_MENU, [canvas](wxCommandEvent&) { canvas->EnableCircleMode(); }, ID_MODE_CIRCLE);
    frame->Bind(wxEVT_MENU, [canvas](wxCommandEvent&) { canvas->EnableSquareMode(); }, ID_MODE_SQUARE);  // Square mode binding

    frame->Show();
    return true;
}
