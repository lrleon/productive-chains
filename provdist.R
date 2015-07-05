
require(stringdist)

dist <- function (names, th) {

    l <- length(names)
    m <- as.integer(l/50) + 1
    message("length = ", l, " m = ", m)
    pairs <- data.frame()
    for (i in seq(l)) {

        if (i %% m == 0)
            message("Progress ", i, " ", l, " ", (i/l)*100, " % ",
                    nrow(pairs), " pairs found")

        ni <- names[i]
        if (is.na(ni))
            next

        for (j in seq(i + 1, l)) {

            nj <- names[j]
            if (is.na(nj))
                next

            d <- stringdist(ni, nj)
            if (d >= th)
                pairs <- rbind(pairs, c(i, j, d))
        }
    }

    message("Finished ", nrow(pairs), " pairs found")

    pairs
}


