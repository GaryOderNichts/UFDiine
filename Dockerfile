FROM wiiuenv/devkitppc:20220806

COPY --from=wiiuenv/libmocha:20220903 /artifacts $DEVKITPRO

WORKDIR /project
