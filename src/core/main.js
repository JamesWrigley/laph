/*************************** Copyrights and License *******************************
 *                                                                                *
 * This file is part of Laph. http://github.com/JamesWrigley/laph/                *
 *                                                                                *
 * Laph is free software: you can redistribute it and/or modify it under          *
 * the terms of the GNU General Public License as published by the Free Software  *
 * Foundation, either version 3 of the License, or (at your option) any later     *
 * version.                                                                       *
 *                                                                                *
 * Laph is distributed in the hope that it will be useful, but WITHOUT ANY        *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS      *
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. *
 *                                                                                *
 * You should have received a copy of the GNU General Public License along with   *
 * Laph. If not, see <http://www.gnu.org/licenses/>.                              *
 *                                                                                *
 *********************************************************************************/

function drawGrid(ctx, width, height, scale, xOffset, yOffset) {
    var sep = 50
    xOffset = scale * xOffset
    yOffset = scale * yOffset

    function transform(pos, offset, center, dimension) {
        var new_pos = pos + offset
        var border = Math.ceil(dimension / sep) * sep

        if (new_pos > border) {
            new_pos %= border
        } else if (new_pos < 0) {
            new_pos = border - Math.abs(new_pos) % border
        }

        return new_pos
    }

    ctx.fillRect(0, 0, width, height)

    var z = Math.max(width, height)
    for (var i = 0; i < z; i += sep) {
        var x = transform(i, xOffset, width / 2, width)
        var y = transform(i, yOffset, height / 2, height)

        // Vertical lines
        if (i < width) {
            ctx.moveTo(x, 0)
            ctx.lineTo(x, height)
        }

        // Horizontal lines
        if (i < height) {
            ctx.moveTo(0, y)
            ctx.lineTo(width, y)
        }
    }
}
