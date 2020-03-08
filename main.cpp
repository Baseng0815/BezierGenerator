#include <cmath>
#include <string>
#include <vector>
#include <stdlib.h>
#include <iostream>

#include <SFML/Graphics.hpp>

#define PI 3.14159265358979

template<typename T>
std::ostream& operator<<(std::ostream& os, const sf::Vector2<T>& vec) {
    os << vec.x << "\t" << vec.y << std::endl;
    return os;
}

class BoxSelection {
    private:
        sf::RectangleShape m_rect;

    public:
        BoxSelection() {
            m_rect.setFillColor(sf::Color(127, 127, 127, 200));
        }

        void startSelection(int x, int y) {
            m_rect.setPosition(x, y);
            m_rect.setSize(sf::Vector2f(0, 0));
        }

        void updateSelection(int x, int y) {
            m_rect.setSize(sf::Vector2f(x - m_rect.getPosition().x, y - m_rect.getPosition().y));
        }

        void endSelection(const std::vector<sf::CircleShape>& circles, std::vector<unsigned int>& selectedCircles) {
            for (unsigned int circle = 0; circle < circles.size(); circle++) {
                if (m_rect.getGlobalBounds().contains(circles[circle].getPosition()))
                    selectedCircles.push_back(circle);
            }

            m_rect.setSize(sf::Vector2f(0, 0));
        }

        void draw(sf::RenderTarget& renderTarget) {
            renderTarget.draw(m_rect);
        }
};

class Circles {
    private:
        std::vector<sf::CircleShape> m_circles;
        int m_draggedCircleIndex = -1;
        bool m_wasChanged = true;

        BoxSelection m_boxSelection;
        bool m_isBoxSelecting = false;
        std::vector<unsigned int> m_draggedCircleIndices;

        sf::Font m_font;
        sf::Text m_text;

        sf::RectangleShape m_lineConnector;

    public:
        Circles(int count) noexcept {
            m_circles = std::vector<sf::CircleShape>(count);
            for (unsigned int circle = 0; circle < m_circles.size(); circle++) {
                m_circles[circle] = sf::CircleShape(10);
                m_circles[circle].setOutlineThickness(-2);
                m_circles[circle].setOutlineColor(sf::Color::Black);
                m_circles[circle].setOrigin(10, 10);

                float angle = 2 * PI / count * circle;
                m_circles[circle].setPosition(400 + 200 * std::sin(angle), 300 + 200 * std::cos(angle));
            }

            m_font.loadFromFile("Hack-Regular.ttf");
            m_text.setFont(m_font);
            m_text.setCharacterSize(20);
            m_text.setFillColor(sf::Color::Black);
            m_text.setOrigin(0, 10);

            m_lineConnector.setFillColor(sf::Color(127, 127, 127, 127));
            m_lineConnector.setSize(sf::Vector2f(0, 6));
            m_lineConnector.setOrigin(0, 3);
        }

        std::vector<sf::Vector2f> getPoints() const {
            std::vector<sf::Vector2f> points;
            for (const auto& circle : m_circles)
                points.push_back(circle.getPosition());
            return points;
        }

        bool wasChanged() {
            bool changed = m_wasChanged;
            m_wasChanged = false;
            return changed;
        }

        void handleEvent(const sf::Event& event) {
            switch (event.type) {
                // mouse press
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        // find circle which was clicked
                        for (unsigned int circle = 0; circle < m_circles.size(); circle++) {
                            if (m_circles[circle].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                                m_draggedCircleIndex = circle;
                                return;
                            }
                        }
                        // if no circle was found (aka click into void), start box selection if it has not been done yet
                        m_boxSelection.startSelection(event.mouseButton.x, event.mouseButton.y);
                        m_isBoxSelecting = true;
                    // reset selected circles on right click
                    } else if (event.mouseButton.button == sf::Mouse::Right) {
                        if (!m_isBoxSelecting) {
                            m_draggedCircleIndices.clear();
                            for (unsigned int circle = 0; circle < m_circles.size(); circle++)
                                m_circles[circle].setFillColor(sf::Color::White);
                        }
                    }
                    break;

                // mouse release
                case sf::Event::MouseButtonReleased:
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        if (m_isBoxSelecting) {
                            m_boxSelection.endSelection(m_circles, m_draggedCircleIndices);
                            for (unsigned int index : m_draggedCircleIndices)
                                m_circles[index].setFillColor(sf::Color(127, 127, 127, 127));

                            m_isBoxSelecting = false;
                        } else {
                            m_draggedCircleIndex = -1;
                        }
                    }

                // mouse move
                case sf::Event::MouseMoved:
                    if (m_draggedCircleIndex != -1) {
                        m_wasChanged = true;
                        if (m_draggedCircleIndices.size()) {
                            int dx = event.mouseMove.x - m_circles[m_draggedCircleIndex].getPosition().x;
                            int dy = event.mouseMove.y - m_circles[m_draggedCircleIndex].getPosition().y;

                            for (unsigned int index : m_draggedCircleIndices)
                                m_circles[index].move(dx, dy);
                        }

                        m_circles[m_draggedCircleIndex].setPosition(event.mouseMove.x, event.mouseMove.y);
                    } else if (m_isBoxSelecting) {
                        m_boxSelection.updateSelection(event.mouseMove.x, event.mouseMove.y);
                    }
                    break;

                default:
                    break;
            }
        }

        void draw(sf::RenderTarget& renderTarget) {
            // circles and text
            for (unsigned int circle = 0; circle < m_circles.size(); circle++) {
                renderTarget.draw(m_circles[circle]);
                m_text.setPosition(m_circles[circle].getPosition().x + 13, m_circles[circle].getPosition().y);
                m_text.setString("P" + std::to_string(circle));
                renderTarget.draw(m_text);
            }

            // line connectors
            for (unsigned int circle = 1; circle < m_circles.size(); circle++) {
                m_lineConnector.setPosition(m_circles[circle].getPosition());

                float dx = m_circles[circle].getPosition().x - m_circles[circle - 1].getPosition().x;
                float dy = m_circles[circle].getPosition().y - m_circles[circle - 1].getPosition().y;
                m_lineConnector.setSize(sf::Vector2f(std::sqrt(dx * dx + dy * dy), 6));
                float angle = std::atan(dy / dx) * 180.f / PI;
                if (dx > 0) angle = angle - 180;
                m_lineConnector.setRotation(angle);
                renderTarget.draw(m_lineConnector);
            }

            // box selection
            if (m_isBoxSelecting)
                m_boxSelection.draw(renderTarget);
        }
};

// t in [0,1]
// done by recursively decreasing degree (cubic -> quadratic -> linear)
sf::Vector2f getBezierPoint(std::vector<sf::Vector2f> points, float t) {
    if (points.size() != 2) {
        std::vector<sf::Vector2f> newPoints;
        for (unsigned int i = 1; i < points.size(); i++)
            newPoints.push_back(points[i - 1] + t * (points[i] - points[i - 1]));
        return getBezierPoint(newPoints, t);
    } else {
        return points[0] + t * (points[1] - points[0]);
    }
}

int main() {
    std::cout << "(1) linear bezier curve\n"
              << "(2) quadratic bezier curve\n"
              << "(3) cubic bezier curve\n"
              << "(4) for other degrees" << std::endl;

    int selection = 0;
    std::cin >> selection;
    if (selection == 4) {
        std::cout << "Enter degree of curve (1 ^= linear...)" << std::endl;
        std::cin >> selection;
    }

    Circles circles(selection + 1);
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Window");

    sf::CircleShape bezierShape(8);
    bezierShape.setOrigin(8, 8);
    bezierShape.setFillColor(sf::Color::Blue);
    std::vector<sf::Vector2f> bezierPoints;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            circles.handleEvent(event);

            switch (event.type) {
                // window close
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Key::Escape) {
                        window.close();
                    }
                    break;

                default:
                   break;
            }
        }

        if (circles.wasChanged()) {
            auto points = circles.getPoints();
            bezierPoints.clear();
            for (float t = 0; t <= 1; t += 0.001) {
                bezierPoints.push_back(getBezierPoint(points, t));
            }
        }

        window.clear(sf::Color::White);
        circles.draw(window);
        for (sf::Vector2f& point : bezierPoints) {
            bezierShape.setPosition(point);
            window.draw(bezierShape);
        }
        window.display();
    }

    return 0;
}
