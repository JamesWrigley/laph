function drawGrid(ctx, width, height, scale, xOffset, yOffset) {
    var sep = 50

    function transform(pos, offset, center, dimension) {
        var new_pos = pos + offset // - (1 - scale) * (pos - center)
        var border = Math.ceil(dimension / sep) * sep 

        if (new_pos > border) {
            new_pos %= border
        } else if (new_pos < 0) {
            new_pos = border - Math.abs(new_pos) % border
        }

        return new_pos
    }

    ctx.fillRect(0, 0, width, height)

    var z = Math.max(width, height) // / scale
    for (var i = 0; i < z; i += sep) {
        var x = transform(i, xOffset, width / 2, width)
        var y = transform(i, yOffset, height / 2, height)

        // Vertical lines
        if (i < width) { // / scale) {
            ctx.moveTo(x, 0)
            ctx.lineTo(x, height)
        }

        // Horizontal lines
        if (i < height) { // / scale) {
            ctx.moveTo(0, y)
            ctx.lineTo(width, y)
        } 
    }
}
