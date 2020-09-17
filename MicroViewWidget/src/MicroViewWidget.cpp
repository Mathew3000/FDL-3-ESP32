#include "MicroViewWidget.h"


uint8_t getInt16PrintLen(int16_t val) {
	int16_t i;
	uint8_t count;

	if (val >= 10000) {
		return 5;
	}

	if (val <= -10000) {
		return 6;
	}

	if (val >= 0) {
		count = 1;
	}
	else {
		count = 2;
		val = abs(val);
	}

	i = 10;
	while (val >= i) {
		count++;
		i *= 10;
	}

	return count;
}

// -------------------------------------------------------------------------------------
// MicroViewWidget Class - start
// -------------------------------------------------------------------------------------

/** \brief MicroView widget parent class.
	The MicroViewWidget class is the parent class for child widget like MicroViewSlider and MicroViewGauge.
*/
MicroViewWidget::MicroViewWidget(uint8_t newx, uint8_t newy, int16_t min, int16_t max):
	posX(newx),
	posY(newy),
	minValue(min),
	maxValue(max),
	value(min)
{
	valLen=getInt16PrintLen(value);
	setMaxValLen();
}

/** \brief Get widget x position. */
uint8_t MicroViewWidget::getX() { return posX; }

/** \brief Get widget y position. */
uint8_t MicroViewWidget::getY() { return posY; }

/** \brief Set widget x position. */
void MicroViewWidget::setX(uint8_t newx) { posX = newx; }

/** \brief Set widget y position. */
void MicroViewWidget::setY(uint8_t newy) { posY = newy; }

/** \brief Get minimum value.
	Return the minimum value of the widget.
*/
int16_t MicroViewWidget::getMinValue() { return minValue; }

/** \brief Get maximum value.
	Return the maximum value of the widget.
*/
int16_t MicroViewWidget::getMaxValue() { return maxValue; }

/** \brief Get current value.
	Return the current value of the widget.
*/
int16_t MicroViewWidget::getValue() { return value; }

/** \brief Set minimum value.
	The minimum value of the widget is set to the variable passed in.
*/
void MicroViewWidget::setMinValue(int16_t min) {
	minValue = min;
	setMaxValLen();
}

/** \brief Set maximum value.
	The maximum value of the widget is set to the variable passed in.
*/
void MicroViewWidget::setMaxValue(int16_t max) {
	maxValue = max;
	setMaxValLen();
}

/** \brief Get the maximum possible print length of the value
	Return the maximum number of characters that would be printed using oled.print(value) for the current value range.
*/
uint8_t MicroViewWidget::getMaxValLen() { return maxValLen; }

/** \brief Set current value.
	The current value of the widget is set to the variable passed in.
*/
void MicroViewWidget::setValue(int16_t val) {
	if ((val<=maxValue) && (val>=minValue)) {
		value = val;
		valLen = getInt16PrintLen(val);
		this->draw();
	}
}

/** \brief Get the print length of the value
	Return the number of characters that would be printed using oled.print(value) for the current value.
*/
uint8_t MicroViewWidget::getValLen() { return valLen; }

/** \brief MicroView Widget reDraw routine.
	Redraws the widget.
*/
void MicroViewWidget::reDraw() {
	this->drawFace();
	this->drawPointer(); // initial pointer (will be erased)
	this->draw();
}

/** \brief Draw a signed decimal numeric value at the current cursor location.
	The value is right justified with leading spaces, within a field the length of the maximum posible for the widget's value range.
*/
void MicroViewWidget::drawNumValue(int16_t value) {

	for (uint8_t i = maxValLen - getInt16PrintLen(value); i > 0; i--) {
		oled.print(" ");
	}
	oled.print(value);
}

/*	Set the maximum number of characters that would be printed
	using oled.print(value) for the current value range.
*/
void MicroViewWidget::setMaxValLen() {
	uint8_t minLen, maxLen;

	maxLen = getInt16PrintLen(maxValue);
	minLen = getInt16PrintLen(minValue);
	maxValLen = maxLen >= minLen ? maxLen : minLen;
}

// -------------------------------------------------------------------------------------
// MicroViewWidget Class - end
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// Slider Widget - start
// -------------------------------------------------------------------------------------

/** \brief MicroViewSlider class initialisation.
	Initialise the MicroViewSlider widget with default style.
*/
MicroViewSlider::MicroViewSlider(uint8_t newx, uint8_t newy, int16_t min, int16_t max):
	MicroViewWidget(newx, newy, min, max),
	style(0),
	totalTicks(30),
	noValDraw(false),
	prevValue(value)
{
	drawFace();
	drawPointer(); // Initial pointer (will be erased)
	draw();
}

/** \brief MicroViewSlider class initialisation with style.
	Initialise the MicroViewSlider widget with style WIDGETSTYLE0 or WIDGETSTYLE1 or WIDGETSTYLE2 (like 0, but vertical) or WIDGETSTYLE3 (like 1, but vertical).
	Add WIDGETNOVALUE to the style to suppress displaying the numeric value. E.g. WIDGETSTYLE0 + WIDGETNOVALUE
*/
MicroViewSlider::MicroViewSlider(uint8_t newx, uint8_t newy, int16_t min, int16_t max, uint8_t sty):
	MicroViewWidget(newx, newy, min, max),
	prevValue(value)
{
	noValDraw = sty & WIDGETNOVALUE; // set "no value draw" flag as specified

	switch(sty & ~WIDGETNOVALUE) {
		case WIDGETSTYLE1:
			style=1;
			totalTicks=60;
			break;
		case WIDGETSTYLE2:
			style=2;
			totalTicks=20;
			break;
		case WIDGETSTYLE3:
			style=3;
			totalTicks=40;
			break;
		default:
			style=0;
			totalTicks=30;
			break;
	}

	drawFace();
	drawPointer(); // Initial pointer (will be erased)
	draw();
}

/** \brief Draw widget face.
	Draw image/diagram representing the widget's face.
*/
void MicroViewSlider::drawFace() {
	uint8_t endOffset;

	//Horizontal styles, style 0 or 1
	if (style==0 || style==1) {
		endOffset = posX + totalTicks + 2;
		// Draw minor ticks
		for (uint8_t i=posX+1; i<endOffset; i+=2) {
			oled.drawFastVLine(i, posY+5, 3, WHITE);
		}
		// Draw extensions for major ticks
		for (uint8_t i=posX+1; i<endOffset; i+=10) {
			oled.drawFastVLine(i, posY+3, 2, WHITE);
		}
	}
	//Vertical styles, style 2 or 3
	else {
		endOffset = posY + totalTicks + 2;
		// Draw minor ticks
		for (uint8_t i=posY+1; i<=endOffset; i+=2) {
			oled.drawFastHLine(posX, i, 3, WHITE);
		}
		// Draw extensions for major ticks
		for (uint8_t i=posY+1; i<=endOffset; i+=10) {
			oled.drawFastHLine(posX+3, i, 2, WHITE);
		}
	}
}

/** \brief Draw widget value.
	Convert the current value of the widget and draw the ticker representing the value.
*/
void MicroViewSlider::draw() {
	drawPointer(); // Erase the previous pointer
	prevValue=value;
	drawPointer(); // Draw the current pointer

	// Draw numeric value if required
	if (!noValDraw) {
		switch(style) {
			case 0:
				oled.setCursor(posX+totalTicks+4, posY+1);
				break;
			case 1:
				oled.setCursor(posX, posY+10);
				break;
			case 2:
				oled.setCursor(posX+1, posY+totalTicks+4);
				break;
			default:
				oled.setCursor(posX+9, posY);
				break;
		}

		drawNumValue(prevValue);
	}
}

// Use XOR mode to erase or draw the pointer for prevValue
void MicroViewSlider::drawPointer() {
	uint8_t tickPosition;

	if (style==0 || style==1) {  // Horizontal
		tickPosition = ((float)(uint16_t)(prevValue-minValue)/(float)(uint16_t)(maxValue-minValue))*totalTicks;
		oled.drawFastHLine(posX+tickPosition, posY, 3, INVERSE);
		oled.drawPixel(posX+1+tickPosition, posY+1, INVERSE);
	}
	else {  // Vertical
		tickPosition = ((float)(uint16_t)(maxValue-prevValue)/(float)(uint16_t)(maxValue-minValue))*totalTicks;
		oled.drawFastVLine(posX+7, posY+tickPosition, 3, INVERSE);
		oled.drawPixel(posX+6, posY+1+tickPosition, INVERSE);
	}
}

// -------------------------------------------------------------------------------------
// Slider Widget - end
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// Gauge Widget - start
// -------------------------------------------------------------------------------------

/** \brief MicroViewGauge class initilisation.
	Initialise the MicroViewGauge widget with default style.
*/
MicroViewGauge::MicroViewGauge(uint8_t newx, uint8_t newy, int16_t min, int16_t max):
	MicroViewWidget(newx, newy, min, max),
	style(0),
	radius(15),
	noValDraw(false),
	prevValue(value)
{
	drawFace();
	drawPointer(); // Initial pointer (will be erased)
	draw();
}

/** \brief MicroViewGauge class initialisation with style.
	Initialise the MicroViewGauge widget with style WIDGETSTYLE0 or WIDGETSTYLE1.
	Add WIDGETNOVALUE to the style to suppress displaying the numeric value. E.g. WIDGETSTYLE0 + WIDGETNOVALUE
*/
MicroViewGauge::MicroViewGauge(uint8_t newx, uint8_t newy, int16_t min, int16_t max, uint8_t sty):
	MicroViewWidget(newx, newy, min, max),
	prevValue(value)
{
	noValDraw = sty & WIDGETNOVALUE; // set "no value draw" flag as specified

	if ((sty & ~WIDGETNOVALUE) == WIDGETSTYLE0) {
		style=0;
		radius=15;
	}
	else {
		style=1;
		radius=23;
	}

	drawFace();
	drawPointer(); // Initial pointer (will be erased)
	draw();
}

/** \brief Draw widget face.
	Draw image/diagram representing the widget's face.
*/
void MicroViewGauge::drawFace() {
	float degreeSec, fromSecX, fromSecY, toSecX, toSecY;

	oled.drawCircle(posX, posY, radius, WHITE);

	for (int i=150;i<=390;i+=30) {	// Major tick from 150 degree to 390 degree
		degreeSec=i*(PI/180);
		fromSecX = cos(degreeSec) * (radius / 1.5);
		fromSecY = sin(degreeSec) * (radius / 1.5);
		toSecX = cos(degreeSec) * radius;
		toSecY = sin(degreeSec) * radius;
		oled.drawLine(1+posX+fromSecX, 1+posY+fromSecY, 1+posX+toSecX, 1+posY+toSecY, WHITE);
	}

	if (radius>15) {
		for (int i=165;i<=375;i+=30) {	// Minor tick from 165 degree to 375 degree
			degreeSec=i*(PI/180);
			fromSecX = cos(degreeSec) * (radius / 1.3);
			fromSecY = sin(degreeSec) * (radius / 1.3);
			toSecX = cos(degreeSec) * radius;
			toSecY = sin(degreeSec) * radius;
			oled.drawLine(1+posX+fromSecX, 1+posY+fromSecY, 1+posX+toSecX, 1+posY+toSecY, WHITE);
		}
	}
}

/** \brief Draw widget value.
	Convert the current value of the widget and draw the ticker representing the value.
*/
void MicroViewGauge::draw() {
	drawPointer(); // Erase the previous pointer
	prevValue=value;
	drawPointer(); // Draw the current pointer

	// Draw numeric value if required
	if (!noValDraw) {
		uint8_t offsetX = posX - (maxValLen * 3 - 1);  // Offset left of centre to print the value

		if (style > 0)
			oled.setCursor(offsetX, posY+10);
		else
			oled.setCursor(offsetX, posY+11);
		uint8_t x = oled.getCursorX();
		uint8_t y = oled.getCursorY();
		oled.setTextSize(1);
		oled.fillRect(x, y, 21, 7, BLACK);
		drawNumValue(prevValue);
	}
}

// Use XOR mode to erase or draw the pointer for prevValue
void MicroViewGauge::drawPointer() {
	float degreeSec, toSecX, toSecY;

	// total 240 degree in the widget with 150 degree starting point
	degreeSec = (((float)(uint16_t)(prevValue-minValue) / (float)(uint16_t)(maxValue-minValue)
	            * 240) + 150) * (PI / 180);

	toSecX = cos(degreeSec) * (radius / 1.2);
	toSecY = sin(degreeSec) * (radius / 1.2);
	oled.drawLine(posX, posY, 1+posX+toSecX, 1+posY+toSecY, INVERSE);
}

// -------------------------------------------------------------------------------------
// Gauge Widget - end
// -------------------------------------------------------------------------------------