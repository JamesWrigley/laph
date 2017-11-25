function drawGrid(ctx, width, height,
                  scale, xOffset, yOffset) {
    var sep = 50 * scale;

    function transform(pos, offset, dimension) {
        var new_pos = pos + offset
        var cd = dimension + ((sep - dimension % sep)) % sep;

        if (new_pos >= cd) {
            new_pos %= cd;
        } else if (new_pos < 0) {
            new_pos = cd - (Math.abs(new_pos) % cd);
        }

        return new_pos;
    }

    for (var i = 0; i < Math.max(width, height); i += sep) {
        var x = transform(i, xOffset, width);
        var y = transform(i, yOffset, height);

        // Vertical lines
        ctx.moveTo(x, 0);
        ctx.lineTo(x, height);

        // Horizontal lines
        ctx.moveTo(0, y);
        ctx.lineTo(width, y);
    }
}
